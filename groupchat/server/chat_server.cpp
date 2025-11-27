#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <random>
#include <thread>
#include <vector>
#include <ctime>

#include "log_manager.cpp"
#include "thread_pool.cpp"
#include "group_manager.cpp"
#include "scheduler.cpp"
#include "job.h"
#include "cache.cpp"

#include "../shared/protocol.h"
#include "../shared/utils.h"

LogManager logger("logs/chat_log.txt");


// ---------------------------
// Global Objects
// ---------------------------

GroupCache cache(5);
GroupManager groupManager;
Scheduler scheduler;

// ---------------------------
// Helper Functions
// ---------------------------

// Random burst time generator for RR scheduling
int random_burst() {
    static std::default_random_engine gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(3, 10);
    return dist(gen);
}

void send_packet(int fd, Packet &pkt) {
    write(fd, &pkt, sizeof(pkt));
}

// ---------------------------
// Packet Processing
// ---------------------------

void process_packet(Packet pkt, int client_socket) {

    switch (pkt.type) {

        // ----------------------
        // JOIN GROUP
        // ----------------------
        case MSG_JOIN: {
            logger.log("Client " + std::to_string(client_socket) + 
           " joined group " + std::to_string(pkt.group_id));
            groupManager.join_group(pkt.group_id, client_socket);

            Packet response{};
            response.type = MSG_JOIN;
            strcpy(response.payload, "Joined group.");
            response.payload_len = strlen(response.payload);
            response.checksum = compute_checksum(response);
            send_packet(client_socket, response);

            break;
        }

        // ----------------------
        // SEND MESSAGE
        // ----------------------
        case MSG_SEND: {
            // Create chat message
            Message msg;
            msg.sender = pkt.sender_id;
            msg.group = pkt.group_id;
            msg.text = pkt.payload;
            msg.timestamp = time(nullptr);

            // Store chat message (NOT join messages)
            groupManager.store_message(pkt.group_id, msg);

            logger.log("Group " + std::to_string(pkt.group_id) + 
           ": Client " + std::to_string(pkt.sender_id) + 
           " sent message: " + msg.text);


            // Broadcast to group members
            auto members = groupManager.get_members(pkt.group_id);
            for (int fd : members) {
                Packet out{};
                out.type = SERVER_BROADCAST;
                out.sender_id = pkt.sender_id;
                out.group_id = pkt.group_id;
                strncpy(out.payload, pkt.payload, 256);

                out.payload_len = strlen(out.payload);
                out.checksum = compute_checksum(out);
                send_packet(fd, out);

            }

            break;
        }

        // ----------------------
        // HISTORY
        // ----------------------
       case MSG_HISTORY: {

    std::vector<Message> history;

    // -------------------------
    // CACHE LOOKUP
    // -------------------------
    bool hit = cache.get(pkt.group_id, history);

    if (hit) {
        logger.log("Cache HIT for group " + std::to_string(pkt.group_id));
    } else {
        logger.log("Cache MISS for group " + std::to_string(pkt.group_id));
        history = groupManager.get_history(pkt.group_id);
        cache.put(pkt.group_id, history);
    }

    // -------------------------
    // SEND HISTORY MESSAGES
    // -------------------------
    for (auto &msg : history) {
        Packet out{};
        out.type = MSG_HISTORY;
        out.group_id = msg.group;
        out.sender_id = msg.sender;
        snprintf(out.payload, 256, "%s", msg.text.c_str());
        out.payload_len = strlen(out.payload);
        out.checksum = compute_checksum(out);
        send_packet(client_socket, out);

    }

    logger.log("Sent " + std::to_string(history.size()) +
               " history messages to client FD " +
               std::to_string(client_socket));

    break;
}


        // ----------------------
        // UNKNOWN PACKET
        // ----------------------
        default: {
    Packet error{};
    error.type = MSG_HISTORY; // safe fallback type
    strcpy(error.payload, "Unknown packet.");
    send_packet(client_socket, error);
    break;
}

    }
}

// ---------------------------
// Worker Thread (Round Robin Scheduler)
// ---------------------------

void worker_thread() {
    while (true) {
        // RR scheduler pops the next job
        Job job = scheduler.get_next_job();
        job.task();
    }
}

// ---------------------------
// Client Handler
// ---------------------------

void handle_client(int client_socket) {
    while (true) {
        Packet pkt{};
        ssize_t bytes = read(client_socket, &pkt, sizeof(pkt));

        // -------------------------
        // FIRST: check if client disconnected
        // -------------------------
        if (bytes <= 0) {
            logger.log("Client FD " + std::to_string(client_socket) + " disconnected.");
            close(client_socket);
            return;
        }

        // -------------------------
        // SECOND: validate protocol version
        // -------------------------
        if (pkt.version != PROTOCOL_VERSION) {
            Packet error{};
            error.type = SERVER_SYSTEM;
            strcpy(error.payload, "Protocol version mismatch.");
            error.payload_len = strlen(error.payload);
            error.checksum = compute_checksum(error);
            send_packet(client_socket, error);
            continue;
        }

        // -------------------------
        // THIRD: validate checksum
        // -------------------------
        uint8_t calc = compute_checksum(pkt);
        if (calc != pkt.checksum) {
            Packet error{};
            error.type = SERVER_SYSTEM;
            strcpy(error.payload, "Packet checksum invalid.");
            error.payload_len = strlen(error.payload);
            error.checksum = compute_checksum(error);
            send_packet(client_socket, error);
            continue;
        }

        // -------------------------
        // FOURTH: schedule the job
        // -------------------------
        int burst = random_burst();

        Job job(client_socket, burst, [pkt, client_socket]() {
            process_packet(pkt, client_socket);
        });

        scheduler.add_job(job);
    }
}


// ---------------------------
// Main Server
// ---------------------------

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 10);

    ThreadPool pool(4);

    // Two RR worker threads
    std::thread worker1(worker_thread);
    std::thread worker2(worker_thread);

    std::cout << "Server running with ROUND ROBIN scheduler..." << std::endl;

    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        pool.enqueue([client] {
            handle_client(client);
        });
    }

    worker1.join();
    worker2.join();
    return 0;
}
