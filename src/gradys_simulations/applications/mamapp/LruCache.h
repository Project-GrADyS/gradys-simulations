/*
 * File:   lrucache.hpp
 * Author: Alexander Ponomarev
 *
 * Created on June 20, 2013, 5:09 PM
 */

#ifndef _LRUCACHE_HPP_INCLUDED_
#define _LRUCACHE_HPP_INCLUDED_

#include <unordered_map>
#include <list>
#include <cstddef>
#include <stdexcept>

namespace cache {

template<typename key_t, typename value_t>
class lru_cache {
public:
    struct node_t {
        key_t key;
        value_t value;
        long expiry;
    };

    typedef typename std::list<node_t>::iterator list_iterator_t;

    lru_cache(size_t max_size) :
        _max_size(max_size) {
    }

    void put(const key_t& key, const value_t& value, long expiry) {
        auto it = _cache_items_map.find(key);
        node_t node;
        node.key = key;
        node.value = value;
        node.expiry = expiry;
        _cache_items_list.push_front(node);
        if (it != _cache_items_map.end()) {
            _cache_items_list.erase(it->second);
            _cache_items_map.erase(it);
        }
        _cache_items_map[key] = _cache_items_list.begin();

        if (_cache_items_map.size() > _max_size) {
            auto last = _cache_items_list.end();
            last--;
            _cache_items_map.erase(last->key);
            _cache_items_list.pop_back();
        }
    }

    void put(const key_t& key, const value_t& value) {
        put(key, value, 0);
    }

    const value_t& get(const key_t& key, long clock) {
        auto it = _cache_items_map.find(key);
        if (it == _cache_items_map.end()) {
            throw std::range_error("There is no such key in cache");
        } else {
            _cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
            return it->second->value;
        }
    }

    bool exists(const key_t& key, long time) const {
        auto it = _cache_items_map.find(key);

        bool exists = it != _cache_items_map.end();
        long expiry = 0;

        if (exists) {
            expiry = it->second->expiry;
        }

        return exists && time < expiry;
    }

    size_t size() const {
        return _cache_items_map.size();
    }

private:
    std::list<node_t> _cache_items_list;
    std::unordered_map<key_t, list_iterator_t> _cache_items_map;
    size_t _max_size;
};

} // namespace cache

#endif  /* _LRUCACHE_HPP_INCLUDED_ */
