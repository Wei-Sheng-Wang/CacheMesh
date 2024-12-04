#ifndef RECOVERY_H
#define RECOVERY_H

#include "wal.h"
#include "lru.h"

#include <string>

class RecoveryManager {
public:
    RecoveryManager(const std::string& wal_path);
    void recoverFromWAL(const std::string& node_id, LRUCache<std::string, std::string>& cache);
private:
    const std::string& wal_path_;
};
#endif
