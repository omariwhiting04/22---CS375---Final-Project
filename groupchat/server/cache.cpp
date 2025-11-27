#include <unordered_map>
#include <list>
#include <mutex>

template<typename Key, typename Value>
class LRUCache {
private:
    size_t capacity;
    std::list<std::pair<Key, Value>> items;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> index;
    std::mutex lock;

public:
    LRUCache(size_t cap) : capacity(cap) {}

    void put(Key k, Value v) {
        std::lock_guard<std::mutex> guard(lock);

        if (index.count(k)) {
            items.erase(index[k]);
        } else if (items.size() >= capacity) {
            auto last = items.back();
            index.erase(last.first);
            items.pop_back();
        }

        items.push_front({k, v});
        index[k] = items.begin();
    }

    bool get(Key k, Value &v) {
        std::lock_guard<std::mutex> guard(lock);

        if (!index.count(k))
            return false;

        auto it = index[k];
        v = it->second;

        items.erase(it);
        items.push_front({k, v});
        index[k] = items.begin();

        return true;
    }
};
