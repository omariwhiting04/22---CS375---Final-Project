#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include "../shared/protocol.h"

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);

    if (connect(sock, (sockaddr*)&serv, sizeof(serv)) < 0) {
        std::cout << "Connection failed.\n";
        return 1;
    }

    std::cout << "Connected to server. Commands:\n";
    std::cout << "/join <group>\n";
    std::cout << "/send <msg>\n";
    std::cout << "/history <group>\n";

    int current_group = 1;   // ⭐ The active group YOU are in

    while (true) {
        std::string input;
        std::getline(std::cin, input);

        Packet pkt{};

        // -----------------------------
        // PARSE COMMANDS
        // -----------------------------
        if (input.rfind("/join", 0) == 0) {
            pkt.type = MSG_JOIN;

            // Extract group number
            current_group = stoi(input.substr(6));     // ⭐ UPDATE ACTIVE GROUP
            pkt.group_id = current_group;

            strcpy(pkt.payload, "join");
        } 
        
        else if (input.rfind("/send", 0) == 0) {
            pkt.type = MSG_SEND;

            // Extract message
            std::string msg = input.substr(6);
            strcpy(pkt.payload, msg.c_str());

            // ⭐ Use active group
            pkt.group_id = current_group;
        } 
        
        else if (input.rfind("/history", 0) == 0) {
            pkt.type = MSG_HISTORY;
            pkt.group_id = stoi(input.substr(9));
        } 
        
        else {
            std::cout << "Unknown command.\n";
            continue;
        }

        // -----------------------------
        // FINALIZE BINARY PACKET
        // -----------------------------
        pkt.version = PROTOCOL_VERSION;
        pkt.sender_id = 1;
        pkt.payload_len = strlen(pkt.payload);
        pkt.checksum = compute_checksum(pkt);

        write(sock, &pkt, sizeof(pkt));

        // --------------------------------
        // RECEIVE SERVER RESPONSE
        // --------------------------------
        Packet response{};
        ssize_t n = read(sock, &response, sizeof(response));

        if (n > 0) {
            // Validate checksum
            uint8_t calc = compute_checksum(response);
            if (calc != response.checksum) {
                std::cout << "[ERROR] Invalid server checksum.\n";
                continue;
            }

            if (response.type == SERVER_BROADCAST) {
                std::cout << "Message from group "
                          << response.group_id
                          << ": " << response.payload << "\n";
            }
            else if (response.type == MSG_HISTORY) {
                std::cout << "(history) " << response.payload << "\n";
            }
            else if (response.type == MSG_JOIN) {
                std::cout << "[system] Joined group.\n";
            }
            else if (response.type == SERVER_SYSTEM) {
                std::cout << "[system] " << response.payload << "\n";
            }
            else {
                std::cout << "[unknown packet type received]\n";
            }
        }
    }

    close(sock);
    return 0;
}
