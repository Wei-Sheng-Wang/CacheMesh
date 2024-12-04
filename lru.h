#ifndef LRU_H
#define LRU_H

#include <unordered_map>
#include <list>
#include <cstddef>
#include <optional>
#include <mutex> 

template <typename K, typename V>
class LRUCache {

private:
    struct CacheItem {
        K key;
        V value;
        std::chrono::steady_clock::time_point expiry;

        CacheItem(const K& k, const V& v, int64_t ttl_seconds): 
            key(k), 
            value(v), 
            expiry(std::chrono::steady_clock::now() + std::chrono::seconds(ttl_seconds))
            {};
    };

    std::size_t capacity_;
    // maps to an iterator to the cache list
    std::unordered_map<K, typename std::list<CacheItem>::iterator> cache_map_;
    // cache list 
    std::list<CacheItem> cache_list_;
    
    std::mutex cache_mutex_;

public:
    LRUCache(std::size_t capacity): capacity_(capacity) {};
    
    // return a copy of the value
    bool get(const K& key, V& value) {
        std::lock_guard<std::mutex> lock(cache_mutex_);

        // iterator for unordered_map
        auto it = cache_map_.find(key);
        // check if end of iterator
        if (it == cache_map_.end()){
            return false;
        }

        auto list_iterator = it->second;

        cache_list_.splice(cache_list_.begin(), cache_list_, list_iterator);

        value = list_iterator->value;
        return true;

    }

    // insert a key-value pair
    void put(const K& key, const V& value, int64_t ttl_seconds = 60){
        std::lock_guard<std::mutex> lock(cache_mutex_);

        if (cache_map_.find(key) == cache_map_.end()){
            // create an iterator? 
            cache_list_.emplace_front(key, value, ttl_seconds);
            cache_map_[key] = cache_list_.begin();
        }else{
            // get the iterator
            auto list_iterator = cache_map_[key];
            list_iterator->value = value;
            // move to front 
            cache_list_.splice(cache_list_.begin(), cache_list_, list_iterator);

        }

        // check if capacity is exceeded 
        if (cache_map_.size() > capacity_){
            auto it = std::prev(cache_list_.end());

        
            cache_map_.erase(it->key);

            cache_list_.pop_back();

        }
        

    }

    // delete a key-value pair
    void remove(const K& key){
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = cache_map_.find(key);
        // first check if key exists
        if (it == cache_map_.end()){
            return;
        }

        cache_list_.erase(it->second);
        cache_map_.erase(it);

    }

    // return the number of elements in the cache
    std::size_t size() const { return cache_map_.size(); }

    // return true if the cache is empty
    bool empty() const { return cache_map_.size() == 0;} ;


    std::mutex& getMutex( ){return this->cache_mutex_; };

    std::unordered_map<K, typename std::list<CacheItem>::iterator>& getCacheMap() { return cache_map_; };


    std::list<CacheItem>& getCacheList()  { return cache_list_; };
};

#endif
