#ifndef WAL_H
#define WAL_H

#include <string>
#include <fstream> 
#include <atomic>
#include <mutex>
#include <string>
#include <chrono>


struct LogEntry {
    enum class OpType: uint8_t {
        PUT,
        REMOVE
    };
    OpType op_type;
    std::string node_id;
    std::string key;
    std::string value;
    int64_t ttl;
    std::chrono::system_clock::time_point timestamp;
    uint64_t sequence_number;

};

class WAL {
private:
    std::string log_path_;
    std::ofstream log_file_;
    // std::atomic<uint64_t> sequence_number_{0};
    std::mutex log_mutex_;


public:
   
    WAL(const std::string& path);
    // bool logPut(const std::string& node_id, const std::string& key, const std::string& value, int64_t ttl);
    // bool logRemove(const std::string& node_id, const std::string& key);
    const std::string& getLogPath() const {return log_path_;}
    static std::string serializeEntry(const std::string& node_id, const LogEntry& entry);
    static LogEntry deserializeEntry(const std::string& data);
     bool writeEntry(const std::string& node_id, LogEntry&& entry);

private:
    static uint32_t calculateCRC32(const std::string& data);
    
};

#endif
