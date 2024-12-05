#ifndef NODE_H
#define NODE_H

#include "lru.h"
#include "consistent_hash.h"
#include "wal.h"
#include "recovery.h"
#include "write_queue.h"
#include "grpcpp/grpcpp.h"
#include "distributed-cache.grpc.pb.h"

#include <mutex>
#include <cstddef>
#include <atomic>
#include <string>
#include <vector>
#include <memory>
#include <thread>


// NEED TO INHERIT LATER
class Node: public distributed_cache::DistributedCache::Service {
private:
    std::string address_;
    std::vector<std::string> peers_;
    std::size_t cache_capacity_;
    std::unique_ptr<LRUCache<std::string, std::string>> lru_cache_;
    std::unique_ptr<grpc::Server> server_;
    ConsistentHash consistent_hash_;
    std::atomic<bool> is_running_;
    std::thread cleanup_thread_;
    std::unique_ptr<WriteQueue> write_queue_;
    std::unique_ptr<RecoveryManager> recovery_manager_;

    std::mutex channel_mutex_;
    std::unordered_map<std::string, std::shared_ptr<grpc::Channel>> channel_pool_;



public:
    Node(
        const std::string& address,
        const std::vector<std::string>& peers,
        std::size_t cache_capacity = 10000,
        const std::string& wal_path = "/Users/wangweisheng/Code/ws/distributed-cache-system-v2/wal.log"
    );
    ~Node();
    void start();
    void stop();

    

     // Override gRPC service methods
    grpc::Status Get(grpc::ServerContext* context, 
                    const distributed_cache::GetRequest* request,
                    distributed_cache::GetResponse* response);
    grpc::Status Put(grpc::ServerContext* context,
                    const distributed_cache::PutRequest* request,
                    distributed_cache::PutResponse* response);
    grpc::Status Remove(grpc::ServerContext* context,
                       const distributed_cache::RemoveRequest* request,
                       distributed_cache::RemoveResponse* response);






private:
        grpc::Status ReplicateToNode(const std::string& node, const std::string& key, const std::string& value, int64_t ttl);
    grpc::Status ForwardPutRequest(const std::string& node,
                    const distributed_cache::PutRequest* request,
                    distributed_cache::PutResponse* response);
    grpc::Status ForwardGetRequest(const std::string& node,
                    const distributed_cache::GetRequest* request,
                    distributed_cache::GetResponse* response);
    grpc::Status ForwardRemoveRequest(const std::string& node,
                    const distributed_cache::RemoveRequest* request,
                    distributed_cache::RemoveResponse* response);

    
    void cleanup();
    std::shared_ptr<grpc::Channel> getOrCreateChannel(const std::string& node_address);




};


#endif