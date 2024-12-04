import grpc
import time
from concurrent import futures
import distributed_cache_pb2_grpc
import distributed_cache_pb2
import random
import statistics
import threading


class CacheClient:
    def __init__(self, address):
        # create a channel to the server
        self.channel = grpc.insecure_channel(address)
        # create a stub to the server
        self.stub = distributed_cache_pb2_grpc.DistributedCacheStub(self.channel)


    def get(self, key):
        try:
            # create a request
            request = distributed_cache_pb2.GetRequest(key=key)
            # send the request to the server
            response = self.stub.Get(request)
            if response.success:
                # print(f"Key {key} found, value: {response.value}")
                return response.value
            else:
                print(f"Key {key} not found")
                return None
        except grpc.RpcError as e:
            print(f"Error getting key {key}: {e}")
            return None

    def put(self, key, value, ttl):
        try:    
            # print(f"Attempting to put key: {key}, value: {value}")
            request = distributed_cache_pb2.PutRequest(key=key, value=value, ttl=ttl)
            response = self.stub.Put(request)
            time.sleep(0.001)
            # print(f"Put response success: {response.success}")
            return response.success
        except Exception as e:
            print(f"Error putting key {key}: {e}")
            return False

    def remove(self, key):
        try:
            request = distributed_cache_pb2.RemoveRequest(key=key)
            response = self.stub.Remove(request)
            return response.success
        except Exception as e:
            print(f"Error removing key {key}: {e}")
            return False

def run_basic_test():
    print("Starting basic cache test")
    # provide the address of the server
    client = CacheClient("localhost:50051")

    # Test 1: Put and get a value
    success = client.put("test_key", "test_value", 10)
    print(f"Put test_key: {success}")

    value = client.get("test_key")
    print(f"Get test_key: {value}")

    assert value == "test_value", "Basic Put/Get test failed"

    # Test 2: Non-existent key
    value = client.get("non_existent_key")
    assert value is None, "Non-existent key should return None"

    # Test 3: TTL expiration
    success = client.put("ttl_key", "ttl_value", 1)
    print("Waiting for TTL to expire...")
    time.sleep(2)

    value = client.get("ttl_key")
    assert value is None, "TTL expired key should return None"


    # Test 4: Update existing key 
    client.put("update_key", "initial_value", 60)
    client.put("update_key", "updated_value", 60)
    value = client.get("update_key")
    assert value == "updated_value", "Update existing key test failed"


    # Test 5: Remove key
    client.remove("update_key")
    value = client.get("update_key")
    assert value is None, "Remove key test failed"

    print("Basic cache test completed successfully")


def run_load_test(num_operations):
    print("Starting load test")

    client = CacheClient("localhost:50051")

    start_time = time.time() 
    for i in range(num_operations):
        client.put(f"key_{i}", f"value_{i}", 600)
        if i % 100 == 0:
            print(f"Wrote {i} operations")

    write_time = time.time() - start_time
    print(f"Write test completed in {write_time} seconds")

    # Read test
    start_time = time.time()
    for i in range(num_operations):
        client.get(f"key_{i}")
        if i % 100 == 0:
            print(f"Read {i} operations")

    read_time = time.time() - start_time
    print(f"Read test completed in {read_time} seconds")

    print(f"\n Load test results:\n")
    print(f"Write operations: {num_operations} in {write_time:.2f} seconds")
    print(f"Read operations: {num_operations} in {read_time:.2f} seconds")
    print(f"Write throughput: {num_operations / write_time:.2f} ops/sec")
    print(f"Read throughput: {num_operations / read_time:.2f} ops/sec")
    print(f"Average write time: {(write_time / num_operations) * 1000:.2f} ms/op")
    print(f"Average read time: {(read_time / num_operations) * 1000:.2f} ms/op")


def test_concurrent_access(num_threads=10, num_operations=1000):
    print("\nTesting concurrent access...")

    client = CacheClient("localhost:50051")
    writer_success = []
    reader_success = []
    def writer(thread_id):
        local_success = 0
        for i in range(num_operations):
            key = f"concurrent_key_{thread_id}_{i}"
            success = client.put(key, f"value_{i}", 6000)
            if success:
                local_success += 1
        writer_success.append(local_success)
        print(f"Writer {thread_id} completed with {local_success} successful writes.")
            

    def reader(thread_id):
        local_hits = 0
        for i in range(num_operations):
            key = f"concurrent_key_{thread_id}_{i}"
            value = client.get(key)
            if value is not None:
                local_hits += 1
        reader_success.append(local_hits)
        print(f"Reader {thread_id} completed with {local_hits} hits.")


    start_time = time.time()

    write_threads = []
    # Phase 1: Write data
    for t_id in range(num_threads):
        t = threading.Thread(target=writer, args=(t_id,))

        write_threads.append(t)
        t.start()

    for t in write_threads:
        t.join()

    
    read_threads = []
   # Phase 2: Read data
    for t_id in range(num_threads):
        t = threading.Thread(target=reader, args=(t_id,))
        read_threads.append(t)
        t.start()

    for t in read_threads:
        t.join()

    end_time = time.time()
    total_writes = sum(writer_success)
    total_hits = sum(reader_success)

    print(f"\nConcurrent access test results:\n")
    print(f"Total successful writes: {total_writes}")
    print(f"Total successful hits: {total_hits}")

    print(f"Write success rate: {total_writes / (num_threads * num_operations):.2f}")
    print(f"Hit rate: {total_hits / total_writes:.2f}")

    print(f"Test completed in {end_time - start_time:.2f} seconds")


def run_all_tests():
    run_basic_test()
    run_load_test(10000)
    test_concurrent_access()

if __name__ == "__main__":
    try:
        run_all_tests()
    except Exception as e:
        print(f"An error occurred: {e}")

