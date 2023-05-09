#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <functional>


class Client {
private:
    int socket_fd;
    
    std::string server_ip;
    int server_port;

    char buffer[1024];

public:
    Client(std::string server_ip, int port) {
        socket_fd = socket(
            AF_INET, 
            SOCK_STREAM | SOCK_NONBLOCK ,
            0
        );
        if (socket_fd == -1) {
            std::cerr << "Failed to create socket " << std::endl;
            exit(EXIT_FAILURE);
        }
        this->server_ip = server_ip;
        this->server_port = port;
    }
    
    void Connect() {
        sockaddr_in server_address{};
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = inet_addr(this->server_ip.c_str());
        server_address.sin_port = htons(this->server_port);

        auto res =   connect(
            socket_fd, 
            (struct sockaddr*) &server_address, 
            sizeof(server_address)
        );

        if (res == -1) {
            std::cerr << "Failed connect" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    std::string Request(std::string message, uint timout_seconds = 0) {
        auto res = send(
            socket_fd, 
            message.c_str(), 
            message.length(), 
            0
        );

        if (res == -1) {
            std::cerr << "Failed to send message " << std::endl;
            exit(EXIT_FAILURE);
        }

        if (timout_seconds == 0){
            return "";
        }

        // TODO:
        //  - add timeout here

        int bytes_received = recv(socket_fd, buffer, sizeof(buffer), 0);
        
        if (bytes_received == -1) {
            std::cerr << "Failed to receive response " << std::endl;
            exit(EXIT_FAILURE);
        }

        std::string resp(buffer, bytes_received);
        return resp;
    }

};
