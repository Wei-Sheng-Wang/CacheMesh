#ifndef WRITE_QUEUE_H
#define WRITE_QUEUE_H

#include "wal.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <chrono>
#include <thread>
#include <cstddef>
#include <memory>
#include <vector>

class WriteQueue {
private:
    std::queue<LogEntry> queue_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_{false};
    std::thread flush_thread_;
    WAL wal_;
    std::string node_id_;
    std::atomic<std::size_t> sequence_number_{0};

    const std::size_t batch_size_;
    const std::chrono::milliseconds flush_interval_;

    void processBatch();
    void flushLoop();

public:
    WriteQueue(const std::string& wal_path, const std::string& node_id, std::size_t batch_size = 100, std::chrono::milliseconds flush_interval = std::chrono::milliseconds(10000));
    ~WriteQueue();

    void start();
    void stop();

    void logPut(const std::string& key, const std::string& value, int64_t ttl);
    void logRemove(const std::string& key);
    void enqueue(LogEntry&& op);
    std::size_t size() ;
};
#endif
