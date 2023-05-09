#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <functional>

class Endpoint {
private:
    int socket_fd;
    int client_socket_fd;
    std::string input;
    std::function<void(std::string)> input_callback;
    std::function<void(std::string)> message_callback;

public:
    Endpoint(const char* port) {
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd == -1) {
            std::cerr << "Failed to create socket" << std::endl;
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(std::stoi(port));

        if (bind(socket_fd, (struct sockaddr*) &address, sizeof(address)) == -1) {
            std::cerr << "Failed to bind to port " << port << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    void Connect(const char* ip_address, const char* port) {
        struct sockaddr_in server_address{};
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = inet_addr(ip_address);
        server_address.sin_port = htons(std::stoi(port));

        if (connect(socket_fd, (struct sockaddr*) &server_address, sizeof(server_address)) == -1) {
            std::cerr << "Failed to connect to server" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    void Start() {
        if (listen(socket_fd, 1) == -1) {
            std::cerr << "Failed to listen for incoming connections" << std::endl;
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in client_address{};
        socklen_t client_address_length = sizeof(client_address);
        client_socket_fd = accept(socket_fd, (struct sockaddr*) &client_address, &client_address_length);
        if (client_socket_fd == -1) {
            std::cerr << "Failed to accept incoming connection" << std::endl;
            exit(EXIT_FAILURE);
        }

        std::cout << "connected" << std::endl;
        std::thread receive_thread(&Endpoint::Receive, this);
        receive_thread.detach();
    }

    void Input(std::function<void(std::string)> callback) {
        input_callback = callback;
    }

    void Message(std::function<void(std::string)> callback) {
        message_callback = callback;
    }

    void Send(std::string message) {
        if (send(client_socket_fd, message.c_str(), message.length(), 0) == -1) {
            std::cerr << "Failed to send message" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    void StartInputLoop() {
        std::string line;
        while (std::getline(std::cin, line)) {
            input_callback(line);
            Send(line);
        }
    }

private:
    void Receive() {
        char buffer[1024];
        int bytes_received;

        while (true) {
            bytes_received = recv(client_socket_fd, buffer, sizeof(buffer), 0);
            if (bytes_received == -1) {
                std::cerr << "Failed to receive message" << std::endl;
                exit(EXIT_FAILURE);
            } else if (bytes_received == 0) {
                break;
            }

            std::string message(buffer, bytes_received);
            message_callback(message);
        }

        close(client_socket_fd);
    }
};
