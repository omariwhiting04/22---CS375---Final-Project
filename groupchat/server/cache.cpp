#include <unordered_map>
#include <list>
#include <mutex>
#include "../shared/message.h"

class GroupCache {
private:
    size_t capacity;
    std::mutex lock;

    // LRU: list holds [group -> vector<messages>]
    std::list<std::pair<uint32_t, std::vector<Message>>> lru_list;

    // Map key to LRU list iterator
    std::unordered_map<uint32_t, 
        std::list<std::pair<uint32_t, std::vector<Message>>>::iterator> index;

public:
    GroupCache(size_t cap) : capacity(cap) {}

    // Put group history into cache
    void put(uint32_t group_id, const std::vector<Message> &history) {
        std::lock_guard<std::mutex> guard(lock);

        if (index.count(group_id)) {
            lru_list.erase(index[group_id]);
        } else if (lru_list.size() >= capacity) {
            auto last = lru_list.back().first;
            index.erase(last);
            lru_list.pop_back();
        }

        lru_list.push_front({group_id, history});
        index[group_id] = lru_list.begin();
    }

    // Try to fetch from cache
    bool get(uint32_t group_id, std::vector<Message> &history) {
        std::lock_guard<std::mutex> guard(lock);

        if (!index.count(group_id))
            return false;

        auto it = index[group_id];
        history = it->second;

        // Move to front
        lru_list.erase(it);
        lru_list.push_front({group_id, history});
        index[group_id] = lru_list.begin();
        return true;
    }
};
