#include <cerrno>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <functional>

  
typedef std::function<std::string(std::string)> message_handler;

class Server {
private:
    int my_socket;
    int port;
    message_handler message_callback;

public:
    Server(int port) {
        my_socket = socket(
            AF_INET, 
            SOCK_STREAM ,
            0
        );
        if (my_socket == -1) {
            std::cerr << "Failed to create socket " << std::endl;
            exit(EXIT_FAILURE);
        }

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        auto res = bind(my_socket, (sockaddr*) &address, sizeof(address));
        if (res == -1) {
            std::cerr << "Failed to bind to port " << port << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    void Listen() {

        int res = listen(my_socket, 1);
        if (res == -1) {
            std::cerr << "Failed to listen for incoming connections" << std::endl;
            exit(EXIT_FAILURE);
        }

        sockaddr_in client_address{};
        socklen_t client_address_length = sizeof(client_address);
        
        while (true) {
            int client_socket_fd = accept(
                my_socket, 
                (sockaddr*) &client_address, 
                &client_address_length
            );
            if (client_socket_fd == -1) {
                std::cerr << "Failed to accept incoming connection" << std::endl;
                exit(EXIT_FAILURE);
            }

            std::thread receive_thread(&Server::handler, this, client_socket_fd);
            receive_thread.detach();
        }
    }

    void SetHandler(message_handler callback) {
        message_callback = callback;
    }

private:
    void handler(int socket) {
        char buffer[1024];
        int bytes_received;

        while (true) {
            bytes_received = recv(
                socket, 
                buffer, 
                sizeof(buffer), 
                0
            );
            if (bytes_received == -1) {
                std::cerr << "Failed to receive message" << std::endl;
                exit(EXIT_FAILURE);
            }

            // closed connection
            if (bytes_received == 0) {
                break;
            }

            std::string message(buffer, bytes_received);
            auto resp = message_callback(message);

            if (resp == "") continue;

            // if there is a response return it to the client
            auto res = send(
                socket, 
                resp.c_str(), 
                resp.length(), 
                0
            );

            if (res == -1) {
                std::cerr << "Failed to send response " << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        close(socket);
    }

    void Send(int socket_fd , std::string message){
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
    }
};
