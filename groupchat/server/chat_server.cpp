#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <random>

#include "thread_pool.cpp"
#include "group_manager.cpp"
#include "scheduler.cpp"
#include "job.h"
#include "../shared/protocol.h"
#include "../shared/utils.h"

GroupManager groupManager;
Scheduler scheduler;

// Random burst time generator
int random_burst() {
    static std::default_random_engine gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(3, 10);
    return dist(gen);
}

void send_packet(int fd, Packet &pkt) {
    write(fd, &pkt, sizeof(pkt));
}

void process_packet(Packet pkt, int client_socket) {

    switch (pkt.type) {

        case MSG_JOIN: {
            groupManager.join_group(pkt.group_id, client_socket);

            Packet response{};
            response.type = MSG_JOIN;
            strcpy(response.payload, "Joined group.");
            send_packet(client_socket, response);
            break;
        }

        case MSG_SEND: {
            Message msg;
            msg.sender = pkt.sender_id;
            msg.group = pkt.group_id;
            msg.text = pkt.payload;
            msg.timestamp = time(nullptr);

            groupManager.store_message(pkt.group_id, msg);

            // Broadcast
            auto members = groupManager.get_members(pkt.group_id);
            for (int fd : members) {
                Packet out{};
                out.type = SERVER_BROADCAST;
                out.sender_id = pkt.sender_id;
                out.group_id = pkt.group_id;
                strncpy(out.payload, pkt.payload, 256);
                send_packet(fd, out);
            }
            break;
        }

        case MSG_HISTORY: {
            auto history = groupManager.get_history(pkt.group_id);
            for (auto &msg : history) {
                Packet out{};
                out.type = MSG_HISTORY;
                out.group_id = msg.group;
                out.sender_id = msg.sender;
                snprintf(out.payload, 256, "%s", msg.text.c_str());
                send_packet(client_socket, out);
            }
            break;
        }
    }
}

// Worker thread function for RR jobs
void worker_thread() {
    while (true) {
        if (scheduler.has_job()) {
            Job job = scheduler.get_next_job();
            job.task(); // execute the job's work
        }
        // no sleep needed — jobs are simulated
    }
}

void handle_client(int client_socket) {
    while (true) {
        Packet pkt{};
        ssize_t bytes = read(client_socket, &pkt, sizeof(pkt));

        if (bytes <= 0) {
            close(client_socket);
            return;
        }

        // Wrap work in a job
        int burst = random_burst();
        Job job(client_socket, burst, [pkt, client_socket]() {
            process_packet(pkt, client_socket);
        });

        scheduler.add_job(job);
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 10);

    // Thread pool for client listeners
    ThreadPool pool(4);

    // Launch RR worker threads
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
