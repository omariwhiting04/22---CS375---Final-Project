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

    connect(sock, (sockaddr*)&serv, sizeof(serv));

    std::cout << "Connected to server. Commands:\n";
    std::cout << "/join <group>\n";
    std::cout << "/send <msg>\n";
    std::cout << "/history <group>\n";

    while (true) {
        std::string input;
        std::getline(std::cin, input);

        Packet pkt{};

        if (input.rfind("/join", 0) == 0) {
            pkt.type = MSG_JOIN;
            pkt.group_id = stoi(input.substr(6));
            strcpy(pkt.payload, "join");
        }
        else if (input.rfind("/send", 0) == 0) {
            pkt.type = MSG_SEND;
            pkt.group_id = 1; // default group
            strcpy(pkt.payload, input.substr(6).c_str());
        }
        else if (input.rfind("/history", 0) == 0) {
            pkt.type = MSG_HISTORY;
            pkt.group_id = stoi(input.substr(9));
        }
        else {
            std::cout << "Unknown command.\n";
            continue;
        }

        pkt.sender_id = 1;
        write(sock, &pkt, sizeof(pkt));

        // Receive server output
        Packet response{};
        ssize_t n = read(sock, &response, sizeof(response));

        if (n > 0) {
            std::cout << "Server: " << response.payload << std::endl;
        }
    }

    close(sock);
    return 0;
}
