#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <ctime>

struct Message {
    uint32_t sender;
    uint32_t group;
    std::string text;
    time_t timestamp;
};

#endif
