#include "node.h"
#include <future>


Node::Node(
    const std::string& address,
    const std::vector<std::string>& peers,
    std::size_t cache_capacity,
    const std::string& wal_path 
    ):
    address_(address), 
    peers_(peers),
    cache_capacity_(cache_capacity),
    lru_cache_(std::make_unique<LRUCache<std::string, std::string>>(cache_capacity)),
    consistent_hash_(52),
    write_queue_(std::make_unique<WriteQueue>(wal_path, address)),
    recovery_manager_(std::make_unique<RecoveryManager>(wal_path)){
        // add current nodes to hash ring
        consistent_hash_.addNode(address);
        for(const auto& peer: peers){
            consistent_hash_.addNode(peer);
        }
        // recover from wal
        recovery_manager_->recoverFromWAL(address_, *lru_cache_);

}

void Node::cleanup() {
    // clean up the expired items
    while(is_running_){
        {
            
            std::lock_guard<std::mutex> lock(lru_cache_->getMutex());
            auto now = std::chrono::steady_clock::now();

            // these should be references to avoid copying
            auto& cache_list = lru_cache_->getCacheList();
            auto& cache_map = lru_cache_->getCacheMap();
            auto it = cache_list.begin();

            while (it != cache_list.end()){
                if (it->expiry <= now) {
                    cache_map.erase(it->key);
                    it = cache_list.erase(it);

                }else{
                    ++it;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));

    }
}

void Node::start(){
    is_running_ = true;
    grpc::ServerBuilder builder;
    grpc::ResourceQuota quota;

    quota.SetMaxThreads(12);
    builder.SetResourceQuota(quota);
    builder.AddListeningPort(address_, grpc::InsecureServerCredentials());
    builder.RegisterService(this);
    server_ = builder.BuildAndStart();
    std::cout << "Cache node started at " << address_ << std::endl;

    // pointer to member function
    cleanup_thread_ = std::thread(&Node::cleanup, this);
    

}

void Node::stop(){
    is_running_ = false;
    // if server is running
    if(server_){
        server_->Shutdown();
    }
    if(cleanup_thread_.joinable()){
        // join with main thread
        cleanup_thread_.join();
    }

}


// get the 
grpc::Status Node::Get(grpc::ServerContext* context, const distributed_cache::GetRequest* request, distributed_cache::GetResponse* response) {
    std::string key = request->key();
    std::string value;

    auto responsible_nodes = consistent_hash_.getNodes(request->key(), 3);
    bool is_responsible = std::find(responsible_nodes.begin(), responsible_nodes.end(), address_) != responsible_nodes.end();

    if (is_responsible) {
        bool found = lru_cache_->get(key, value);
        // forward to any of the responsible nodes

        if (found) {
            response->set_value(value);
            response->set_success(true);
            return grpc::Status::OK;
        }
    }else{
        // forward to other nodes if value not found locally 
        return ForwardGetRequest(responsible_nodes[0], request, response);
    
    }

    response->set_success(false);
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "Key not found");

}



grpc::Status Node::Put(grpc::ServerContext* context, const distributed_cache::PutRequest* request, distributed_cache::PutResponse* response) {
    std::vector<std::string> responsible_nodes = consistent_hash_.getNodes(request->key(), 3);

    // check if this node is one of the responsible nodes 
    auto it = std::find(responsible_nodes.begin(), responsible_nodes.end(), address_);
    // Add debug logging
    // std::cout << "Put request received for key: " << request->key() << std::endl;
    // std::cout << "Responsible nodes: ";
    // for (const auto& node : responsible_nodes) {
    //     std::cout << node << " ";
    // }
    // std::cout << "\nCurrent node: " << address_ << std::endl;

    bool is_responsible = it != responsible_nodes.end();
    // std::cout << "Is responsible: " << (is_responsible ? "true" : "false") << std::endl;

    if(!is_responsible){
        // check if no responsible nodes
        if(responsible_nodes.empty()){
            response->set_success(false);
            return grpc::Status(grpc::StatusCode::INTERNAL, "No responsible nodes");
        }
        // forward to any of the responsible nodes
        return ForwardPutRequest(responsible_nodes[0], request, response);

    }

    write_queue_->logPut(request->key(), request->value(), request->ttl());


  
    lru_cache_->put(request->key(), request->value(), request->ttl());

    if(request->is_replica()){
        response->set_success(true);
        return grpc::Status::OK;
    }

    std::vector<std::future<grpc::Status>> replication_futures;

    for(const auto& peer: responsible_nodes){
        if(peer == address_){
            continue;
        }
        replication_futures.push_back(
            std::async(
                std::launch::async,
                &Node::ReplicateToNode,
                this,
                peer,
                request->key(),
                request->value(),
                request->ttl()
            )
        );
    }
    bool all_successful = true;
    // wait for all futures to complete
    for(auto& future: replication_futures){
        grpc::Status status = future.get();
        if(!status.ok()){
            all_successful = false;
        
            std::cout << "Failed to replicate to node: " << status.error_message() << std::endl;
        }
    }
    response->set_success(all_successful);
    return grpc::Status::OK;
}


grpc::Status Node::ReplicateToNode(const std::string& node, const std::string& key, const std::string& value, int64_t ttl) {

    auto channel = getOrCreateChannel(node);
    auto stub = distributed_cache::DistributedCache::NewStub(channel);

    distributed_cache::PutRequest put_request;
    put_request.set_key(key);
    put_request.set_value(value);
    put_request.set_ttl(ttl);
    put_request.set_is_replica(true);
    distributed_cache::PutResponse put_response;

    grpc::ClientContext context;
    return stub->Put(&context, put_request, &put_response);

}


grpc::Status Node::Remove(grpc::ServerContext* context, const distributed_cache::RemoveRequest* request, distributed_cache::RemoveResponse* response) {
    // if (!wal_->logRemove(address_, request->key())){
    //     return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to log remove operation");
    // }
    write_queue_->logRemove(request->key());
 
    lru_cache_->remove(request->key());
    response->set_success(true);
    return grpc::Status::OK;
}


grpc::Status Node::ForwardPutRequest(const std::string& node,
                    const distributed_cache::PutRequest* request,
                    distributed_cache::PutResponse* response){
    auto channel = getOrCreateChannel(node);
    auto stub = distributed_cache::DistributedCache::NewStub(channel);
    grpc::ClientContext client_context;
    return stub->Put(&client_context, *request, response);
}

grpc::Status Node::ForwardGetRequest(const std::string& node,
                    const distributed_cache::GetRequest* request,
                    distributed_cache::GetResponse* response){
    auto channel = getOrCreateChannel(node);
    auto stub = distributed_cache::DistributedCache::NewStub(channel);
    grpc::ClientContext client_context;
    return stub->Get(&client_context, *request, response);
}
grpc::Status Node::ForwardRemoveRequest(const std::string& node,
                    const distributed_cache::RemoveRequest* request,
                    distributed_cache::RemoveResponse* response){
    auto channel = getOrCreateChannel(node);
    auto stub = distributed_cache::DistributedCache::NewStub(channel);
    grpc::ClientContext client_context;
    return stub->Remove(&client_context, *request, response);
}

std::shared_ptr<grpc::Channel> Node::getOrCreateChannel(const std::string& node_address){
    std::lock_guard<std::mutex> lock(channel_mutex_);
    auto it = channel_pool_.find(node_address);
    if(it != channel_pool_.end()){
        return it->second;
    }
    // create a new channel
    auto channel = grpc::CreateChannel(node_address, grpc::InsecureChannelCredentials());
    channel_pool_[node_address] = channel;
    return channel;
}
