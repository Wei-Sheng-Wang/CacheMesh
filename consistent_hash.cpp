#include "consistent_hash.h"
#include <functional>

ConsistentHash::ConsistentHash(std::size_t virtualNodesNum): virtual_nodes_num_(virtualNodesNum) {}


std::size_t ConsistentHash::computeHash(const std::string& key) const{
    std::hash<std::string> hasher;
    return hasher(key) % RING_SPACE_;
}

void ConsistentHash::addNode(const std::string& nodeId) {
    // compute the hash of the nodeId
    std::lock_guard<std::mutex> lock(mutex_);
    for(std::size_t i = 0; i < virtual_nodes_num_; ++i){
        std::string virtualNodeId = nodeId + "#" + std::to_string(i);
        std::size_t hash = computeHash(virtualNodeId);
        hash_ring_[hash] = nodeId;

    }

}

void ConsistentHash::removeNode(const std::string& nodeId) {
    std::lock_guard<std::mutex> lock(mutex_);
    for(std::size_t i = 0; i < virtual_nodes_num_; ++i){
        std::string virtualNodeId = nodeId + "#" + std::to_string(i);
        std::size_t hash = computeHash(virtualNodeId);
        hash_ring_.erase(hash);
    }
}

std::vector<std::string> ConsistentHash::getNodes(const std::string& key, std::size_t replica_count){
    std::lock_guard<std::mutex> lock(mutex_);
    std::size_t hash = computeHash(key);

    std::vector<std::string> nodes;
    // find the first node that is greater than or equal to the hash of the data key
    auto it = hash_ring_.lower_bound(hash);

    while(nodes.size() < replica_count && nodes.size() < hash_ring_.size()){
        if(it == hash_ring_.end()){
            it = hash_ring_.begin();
        }
        // if the nodes vector doesn't contain tis node, then insert it
        if(std::find(nodes.begin(), nodes.end(), it->second) == nodes.end()){
            nodes.push_back(it->second);
        }
        ++it;
    }
    return nodes;
}