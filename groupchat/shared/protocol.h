#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>
#include <cstring>

enum PacketType {
    MSG_SEND = 1,
    MSG_HISTORY = 2,
    MSG_JOIN = 3,
    MSG_LEAVE = 4,
    SERVER_BROADCAST = 5,
    HEARTBEAT = 6
};

struct Packet {
    uint16_t type;
    uint32_t sender_id;
    uint32_t group_id;
    char payload[256];

    Packet() {
        type = 0;
        sender_id = 0;
        group_id = 0;
        memset(payload, 0, sizeof(payload));
    }
};

#endif
