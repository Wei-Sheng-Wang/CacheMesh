#ifndef CONSISTENT_HASH_H
#define CONSISTENT_HASH_H

#include <map>
#include <string>
#include <vector>
#include <mutex>
#include <cstddef>
#include <limits>


class ConsistentHash {
private:
    // map hash number to node id
    std::map<std::size_t, std::string> hash_ring_;
    // defines the maximum value of the hash ring space, used to ensure the ring wraps around correctly.
    static constexpr std::size_t RING_SPACE_ = std::numeric_limits<std::size_t>::max(); 
    std::size_t virtual_nodes_num_;
    std::mutex mutex_;

public:
    // delete copy and move operations to prevent accidental copying/moving
    ConsistentHash(const ConsistentHash&) = delete;
    ConsistentHash& operator=(const ConsistentHash&) = delete;
    ConsistentHash(ConsistentHash&&) = delete;
    ConsistentHash& operator=(ConsistentHash&&) = delete;

    ConsistentHash(std::size_t virtualNodesNum);
    std::size_t computeHash(const std::string& key) const;

    void addNode(const std::string& nodeId);
    void removeNode(const std::string& nodeId);
    
    // get nodes responsible for a given key
    // decides how many replica user wants to keep
    std::vector<std::string> getNodes(const std::string& key, std::size_t replica_count);

};

#endif // CONSISTENT_HASH_H