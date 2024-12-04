#include "recovery.h"
#include "lru.h"
#include "wal.h"
#include <fstream>
#include <iostream>

RecoveryManager::RecoveryManager(const std::string& wal_path) : wal_path_(wal_path) {}

void RecoveryManager::recoverFromWAL(const std::string& node_id, LRUCache<std::string, std::string>& cache) {
    std::cout << "Starting recovery from WAL..." << std::endl;
    std::size_t entries_recovered = 0;
    
    try {
        // reading in binary mode
        std::ifstream wal_file(wal_path_, std::ios::binary);
        
        while (wal_file.good()) {
            try {
                // Read the entry length 
                uint32_t length;
                wal_file.read(reinterpret_cast<char*>(&length), sizeof(length));
                if(wal_file.eof()) break;

                // Read the entry data
                std::vector<char> buffer(length);
                wal_file.read(buffer.data(), length);

                // Deserialize the entry
                LogEntry entry = WAL::deserializeEntry(std::string(buffer.begin(), buffer.end()));

                auto now = std::chrono::system_clock::now();
                
                if (entry.timestamp + std::chrono::seconds(entry.ttl) > now && entry.node_id == node_id) {
                    // Apply the operation to the cache
                    if (entry.op_type == LogEntry::OpType::PUT) {
                        cache.put(entry.key, entry.value, entry.ttl);
                    } else if (entry.op_type == LogEntry::OpType::REMOVE) {
                        cache.remove(entry.key);
                    }
                    entries_recovered++;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error during entry recovery: " << e.what() << std::endl;
                continue;  // Skip to next entry on error
            }
        }
        
        std::cout << "Recovery completed: " << entries_recovered << " entries processed" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error during recovery: " << e.what() << std::endl;
        throw;  // Re-throw fatal errors
    }
}