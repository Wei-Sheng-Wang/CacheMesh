# Distributed Cache System

A high-performance, distributed in-memory cache system built with C++ and gRPC, featuring consistent hashing, write-ahead logging, and automatic recovery mechanisms.

## Features

- **Distributed Architecture**: Supports multiple cache nodes with automatic data distribution
- **Consistent Hashing**: Ensures efficient data distribution and rebalancing
- **Write-Ahead Logging (WAL)**: Provides durability and crash recovery
- **LRU Cache**: Implements Least Recently Used eviction policy
- **Replication**: Supports data replication across multiple nodes for fault tolerance
- **gRPC Communication**: High-performance inter-node communication
- **Batch Processing**: Optimized write operations through batching
- **Automatic Recovery**: System state recovery after crashes
- **TTL Support**: Time-to-live functionality for cache entries

## System Architecture

The system consists of several key components:

1. **Node**: Main server component that handles cache operations
2. **LRU Cache**: In-memory cache implementation with LRU eviction
3. **Consistent Hash Ring**: Manages data distribution across nodes
4. **Write-Ahead Log**: Ensures durability of operations
5. **Recovery Manager**: Handles system recovery after failures
6. **Write Queue**: Batches write operations for better performance

## Building the Project

### Prerequisites

- CMake (3.14 or higher)
- C++17 compatible compiler
- gRPC
- Protocol Buffers
- Boost (for CRC32)

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

## Running the System

### Local Deployment (5-Node Setup)

To run a local cluster with 5 nodes, open 5 terminal windows and run the following commands (one in each terminal):

```bash
# Terminal 1
./distributed_cache localhost:50051 localhost:50052 localhost:50053 localhost:50054 localhost:50055

# Terminal 2
./distributed_cache localhost:50052 localhost:50053 localhost:50054 localhost:50055 localhost:50051

# Terminal 3
./distributed_cache localhost:50053 localhost:50054 localhost:50055 localhost:50051 localhost:50052

# Terminal 4
./distributed_cache localhost:50054 localhost:50055 localhost:50051 localhost:50052 localhost:50053

# Terminal 5
./distributed_cache localhost:50055 localhost:50051 localhost:50052 localhost:50053 localhost:50054
```

Each node will start with its own address as the first argument, followed by the addresses of its peers. This creates a ring topology where each node is aware of all other nodes in the system.

### Running Tests

To run the test suite:

```bash
# Run the test suite
python test_client.py
```

The test suite includes:
- Basic operations test:
  - Put/Get/Remove operations
  - TTL expiration verification
  - Non-existent key handling
  - Key updates
- Load testing:
  - Throughput measurement
  - Read/Write performance metrics
- Concurrent access testing:
  - Multi-threaded writes and reads
  - Success rate measurement
  - Cache hit rate verification

Note: The current test suite focuses on basic functionality, performance, and concurrent access. Node failure scenarios and data replication verification are not currently implemented in the automated tests. These scenarios should be tested manually by stopping and starting nodes in the 5-node setup.

## Usage

### Starting a Cache Node

```cpp
// Initialize a cache node
Node node(
    "localhost:50051",           // Node address
    {"localhost:50052"},         // Peer addresses
    10000,                      // Cache capacity
    "path/to/wal.log"           // WAL file path
);
node.start();
```

### Basic Operations

The system supports three main operations:

1. **Put**: Store a key-value pair with optional TTL
2. **Get**: Retrieve a value by key
3. **Remove**: Delete a key-value pair

### Client Usage Example

```python
from test_client import CacheClient

# Create a client
client = CacheClient("localhost:50051")

# Put operation
client.put("key", "value", ttl=3600)  # TTL in seconds

# Get operation
value = client.get("key")

# Remove operation
client.remove("key")
```

## System Components

### Write-Ahead Log (WAL)

The WAL ensures durability by recording all write operations before they are applied to the cache. It supports:
- Batch writing for better performance
- CRC32 checksums for data integrity
- Operation serialization using Protocol Buffers

### Recovery Manager

Handles system recovery after crashes by:
- Reading and validating WAL entries
- Replaying valid operations
- Handling corrupted entries
- Maintaining consistency across nodes

### Consistent Hashing

Manages data distribution with:
- Automatic node addition/removal
- Load balancing
- Minimal data redistribution during topology changes

## Performance Considerations

- Write operations are batched for improved throughput
- Consistent hashing minimizes data movement during scaling
- gRPC server configured with maximum 12 threads per node for request handling
- LRU cache ensures optimal memory usage with background cleanup thread
- Write-ahead logging batches operations for better I/O performance
- Asynchronous replication using dedicated threads for better throughput
- Background write queue processing for optimized disk I/O

## Performance Results

Benchmark results for single-node scenario:
- Write throughput: 3,116.01 operations/second
- Read throughput: 7,384.24 operations/second
- Average write latency: 0.32 ms/operation
- Average read latency: 0.14 ms/operation

The results were obtained by running the test suite on my M3 Max MacBook Pro with 36GB RAM.


## License

This project is licensed under the MIT License - see the LICENSE file for details.
