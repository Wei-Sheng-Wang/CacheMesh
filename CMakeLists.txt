cmake_minimum_required(VERSION 3.14)

project(DistributedCache VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# Specify the path to Homebrew-installed libraries
set(CMAKE_PREFIX_PATH "/opt/homebrew/")

# Find Protobuf package first with explicit paths
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using Protocol Buffers ${Protobuf_VERSION}")
message(STATUS "Protobuf_INCLUDE_DIRS: ${Protobuf_INCLUDE_DIRS}")

find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

# Get the gRPC CPP plugin path
get_target_property(gRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin LOCATION)

# Proto files
set(PROTO_FILES 
    distributed-cache.proto
    wal.proto
)

# Use the correct protobuf generation commands
add_library(proto-objects OBJECT)
protobuf_generate(TARGET proto-objects LANGUAGE cpp PROTOS ${PROTO_FILES})
get_target_property(PROTO_SRCS proto-objects SOURCES)
target_link_libraries(proto-objects PUBLIC protobuf::libprotobuf)
target_include_directories(proto-objects PUBLIC 
    ${Protobuf_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Generate gRPC files
add_library(grpc-objects OBJECT)
protobuf_generate(
    TARGET grpc-objects 
    LANGUAGE grpc 
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc 
    PROTOS ${PROTO_FILES}
    PLUGIN "protoc-gen-grpc=${gRPC_CPP_PLUGIN_EXECUTABLE}"
)
get_target_property(GRPC_SRCS grpc-objects SOURCES)
target_link_libraries(grpc-objects PUBLIC protobuf::libprotobuf gRPC::grpc++)
target_include_directories(grpc-objects PUBLIC 
    ${Protobuf_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Add the executable
add_executable(distributed_cache
    main.cpp
    node.cpp
    consistent_hash.cpp
    wal.cpp
    recovery.cpp
    write_queue.cpp
    $<TARGET_OBJECTS:proto-objects>
    $<TARGET_OBJECTS:grpc-objects>
)

# Link against gRPC, Protobuf, and pthread
target_link_libraries(distributed_cache
    PRIVATE
        proto-objects
        grpc-objects
        protobuf::libprotobuf
        gRPC::grpc++
        Threads::Threads
)

target_include_directories(distributed_cache
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${Protobuf_INCLUDE_DIRS}
        ${gRPC_INCLUDE_DIRS}
        .
)

# Add after line 25 in CMakeLists.txt
set(CMAKE_DISABLE_SOURCE_CHANGES OFF)
set(CMAKE_DISABLE_IN_SOURCE_BUILD OFF)