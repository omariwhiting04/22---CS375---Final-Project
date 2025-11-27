#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>
#include <cstring>

#define PROTOCOL_VERSION 1
#define MAX_PAYLOAD_SIZE 256

// --------------------------------------------------
// Packet Types
// --------------------------------------------------
enum PacketType {
    MSG_SEND = 1,
    MSG_HISTORY = 2,
    MSG_JOIN = 3,
    MSG_LEAVE = 4,
    SERVER_BROADCAST = 5,
    SERVER_SYSTEM = 6,
};

// --------------------------------------------------
// Packet Structure (Binary Protocol)
// --------------------------------------------------
struct Packet {
    uint8_t version;          // protocol version
    uint16_t type;            // type of packet
    uint32_t sender_id;       // sender
    uint32_t group_id;        // group or room
    uint16_t payload_len;     // length of payload data
    uint8_t checksum;         // XOR checksum for validation
    char payload[MAX_PAYLOAD_SIZE];

    Packet() {
        version = PROTOCOL_VERSION;
        type = 0;
        sender_id = 0;
        group_id = 0;
        payload_len = 0;
        checksum = 0;
        memset(payload, 0, sizeof(payload));
    }
};

// --------------------------------------------------
// Checksum Creation
// --------------------------------------------------
inline uint8_t compute_checksum(const Packet &pkt) {
    uint8_t sum = 0;
    sum ^= pkt.version;
    sum ^= pkt.type & 0xFF;
    sum ^= (pkt.type >> 8) & 0xFF;
    sum ^= pkt.sender_id;
    sum ^= pkt.group_id;
    sum ^= pkt.payload_len;
    for (int i = 0; i < pkt.payload_len; i++) {
        sum ^= pkt.payload[i];
    }
    return sum;
}

#endif
