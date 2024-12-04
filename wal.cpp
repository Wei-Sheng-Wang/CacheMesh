#include "wal.h"
#include "wal.pb.h"
#include <boost/crc.hpp> 

WAL::WAL(const std::string& path) : log_path_(path) {
    log_file_.open(log_path_, std::ios::app | std::ios::binary);
    if (!log_file_) {
        std::cerr << "Failed to open WAL file" << std::endl;
        throw std::runtime_error("Failed to open WAL file");
    }
    std::cout << "WAL file opened successfully" << std::endl;

};


// bool WAL::logPut(const std::string& node_id, const std::string& key, const std::string& value, int64_t ttl) {   
//     // create the log entry and write 
//     LogEntry entry {
//         .key = key,
//         .value = value,
//         .ttl = ttl,
//         .sequence_number = ++sequence_number_,
//         .op_type = OperationType::PUT,
//         .timestamp = std::chrono::system_clock::now()

//     };
//     return writeEntry(node_id, entry);

// }

// bool WAL::logRemove(const std::string& node_id, const std::string& key) {
//     LogEntry entry{
//         .key = key,
//         .timestamp = std::chrono::system_clock::now(),
//         .op_type = OperationType::REMOVE,
//         .sequence_number = ++sequence_number_

//     };
//     return writeEntry(node_id, entry);
// }

bool WAL::writeEntry(const std::string& node_id, LogEntry&& entry) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    // serialize the entry
    const std::string& serialized = serializeEntry(node_id, entry);

    // write length prefix
    uint32_t length = serialized.size();

   
    if (!log_file_.write(reinterpret_cast<char*>(&length), sizeof(length))) {
        std::cerr << "Failed to write length prefix to WAL" << std::endl;
        return false;
    }

    if (!log_file_.write(serialized.data(), serialized.size())) {
        std::cerr << "Failed to write serialized data to WAL" << std::endl;
        return false;
    }

  
    log_file_.flush();

    if (log_file_.fail()) {
        std::cerr << "Failed to write to WAL file" << std::endl;
        return false;
    }

    
    return true;
}

uint32_t WAL::calculateCRC32(const std::string& data) {
    boost::crc_32_type result;
    result.process_bytes(data.data(), data.length());
    return result.checksum();
}
    
std::string WAL::serializeEntry(const std::string& node_id, const LogEntry& entry) {
    distributed_cache::WALEntry proto_entry;

    proto_entry.set_sequence_number(entry.sequence_number);
    proto_entry.set_op_type(static_cast<distributed_cache::WALEntry_OperationType>(entry.op_type));
    proto_entry.set_key(entry.key);
    proto_entry.set_value(entry.value);
    proto_entry.set_ttl(entry.ttl);

    // convert timestamp to milliseconds
    proto_entry.set_timestamp(
            std::chrono::duration_cast<std::chrono::milliseconds>(
            entry.timestamp.time_since_epoch()
        ).count()
    );
    proto_entry.set_node_id(node_id);

    std::string serialized = proto_entry.SerializeAsString();
    uint32_t checksum = calculateCRC32(serialized);
    proto_entry.set_checksum(checksum);

    return proto_entry.SerializeAsString();
    
}

LogEntry WAL::deserializeEntry(const std::string& data) {
    distributed_cache::WALEntry proto_entry;
    if (!proto_entry.ParseFromString(data)) {
        throw std::runtime_error("Failed to parse WAL entry");
    }
    
    uint32_t checksum = proto_entry.checksum();
    // verify checksum
    proto_entry.clear_checksum();
    uint32_t calculated_checksum = calculateCRC32(proto_entry.SerializeAsString());

    if (checksum != calculated_checksum) {
        throw std::runtime_error("WAL entry corruption detected");
    }

    LogEntry entry{
        .sequence_number = proto_entry.sequence_number(),
        .op_type = static_cast<LogEntry::OpType>(proto_entry.op_type()),
        .key = proto_entry.key(),
        .value = proto_entry.value(),
        .ttl = proto_entry.ttl(),
        .timestamp = std::chrono::system_clock::time_point(
            std::chrono::milliseconds(proto_entry.timestamp())
        ),
        .node_id = proto_entry.node_id()

    };

    return entry;

}


