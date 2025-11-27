#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <chrono>

inline std::string timestamp() {
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    return std::string(std::ctime(&tt));
}

#endif
