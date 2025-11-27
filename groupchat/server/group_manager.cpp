#include <unordered_map>
#include <vector>
#include <mutex>
#include <algorithm>
#include "../shared/message.h"

class GroupManager {
private:
    std::unordered_map<uint32_t, std::vector<Message>> groups;
    std::unordered_map<uint32_t, std::vector<int>> members; 
    std::mutex lock;

public:

    // Add a client to a group
    void join_group(uint32_t group, int client_fd) {
        std::lock_guard<std::mutex> guard(lock);
        members[group].push_back(client_fd);
    }

    // Remove client from group
    void leave_group(uint32_t group, int client_fd) {
        std::lock_guard<std::mutex> guard(lock);
        auto &vec = members[group];
        vec.erase(std::remove(vec.begin(), vec.end(), client_fd), vec.end());
    }

    // Store message in history
    void store_message(uint32_t group, const Message &msg) {
        std::lock_guard<std::mutex> guard(lock);
        groups[group].push_back(msg);
    }

    std::vector<Message> get_history(uint32_t group) {
        std::lock_guard<std::mutex> guard(lock);
        return groups[group];
    }

    // Get member client sockets for broadcast
    std::vector<int> get_members(uint32_t group) {
        std::lock_guard<std::mutex> guard(lock);
        return members[group];
    }
};
