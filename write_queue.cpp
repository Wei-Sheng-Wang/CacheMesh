#include "write_queue.h"
#include <future>
#include <iostream>

WriteQueue::WriteQueue(const std::string& wal_path, const std::string& node_id, std::size_t batch_size, std::chrono::milliseconds flush_interval)
    : wal_(WAL(wal_path))
    , node_id_(node_id)
    , batch_size_(batch_size)
    , flush_interval_(flush_interval) {}

WriteQueue::~WriteQueue(){
    stop();
}

void WriteQueue::start() {
    running_ = true;
    // start this thread 
    flush_thread_ = std::thread(&WriteQueue::flushLoop, this);

}

void WriteQueue::stop() {
    running_ = false;

    // currently we only have one thread, but may add additional threads in the future 
    // change to notify_all if more threads 
    cv_.notify_one();


    if (flush_thread_.joinable()){
        flush_thread_.join();
    }

}

void WriteQueue::enqueue(LogEntry&& op) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        queue_.push(op); 
    }
    cv_.notify_one(); // notify the flush thread, does cv_ needs the lock? hmmm
}

void WriteQueue::flushLoop() {
    std::vector<LogEntry> batch;

    while (running_) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        std::size_t current_size = queue_.size();


        // since cv_ acquires the lock, it still holds the queue_mutex_ even after passing the cv_.wait call
        cv_.wait_for(lock, flush_interval_, [this, current_size] {return !running_ || current_size >= batch_size_;});

        if (!running_ && queue_.empty()) {
            break;
        }
        lock.unlock();
   
        // process by batch
        processBatch();
        // lock.lock();

    }
    
    processBatch();
}

void WriteQueue::processBatch() {
    std::vector<LogEntry> batch;
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        while (size() > 0) {
            batch.push_back(std::move(queue_.front()));
            queue_.pop();

        }
    }

    wal_.writeBatch(node_id_, batch);


}   


std::size_t WriteQueue::size() {
    return queue_.size();
}


void WriteQueue::logPut(const std::string& key, const std::string& value, int64_t ttl) {
    // construct the LogEntry first 
    LogEntry entry{
        .key = key,
        .value = value,
        .ttl = ttl,
        .sequence_number = ++sequence_number_,
        .op_type = LogEntry::OpType::PUT,
        .timestamp = std::chrono::system_clock::now()
    };
    enqueue(std::move(entry));

}


void WriteQueue::logRemove(const std::string& key) {
    LogEntry entry{
        .key = key,
        .sequence_number = ++sequence_number_,
        .op_type = LogEntry::OpType::REMOVE,
        .timestamp = std::chrono::system_clock::now()

    };
    enqueue(std::move(entry));
}
