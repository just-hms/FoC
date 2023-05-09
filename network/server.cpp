#include <cerrno>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <functional>

  
#define BUF_LEN 1024

typedef std::function<std::string(std::string)> message_handler;

class Server {
private:
    int sd;
    int port;
    message_handler message_callback;
    char buffer[BUF_LEN];

public:
    
    Server(int port) {
        // create a non blocking socket
        this->sd = socket(
            AF_INET, 
            SOCK_STREAM | SOCK_NONBLOCK,
            0
        );
        if (this->sd == -1) {
            std::cerr << "Failed to create socket " << std::endl;
            exit(EXIT_FAILURE);
        }

        // bind the specified port to the socket
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        auto res = bind(
            this->sd, 
            (sockaddr*) &address, 
            sizeof(address)
        );
        if (res == -1) {
            std::cerr << "Failed to bind to port " << port << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    void Listen() {
        fd_set master, read_fds;
         
        listen(this->sd, 10);

        FD_ZERO(&master);
        FD_ZERO(&read_fds);

        FD_SET(this->sd, &master);

        auto fdmax = sd;          

        while(true){
            read_fds = master;     
        
            select(
                fdmax + 1,
                &read_fds, 
                NULL, 
                NULL, 
                NULL
            );

            for(int i = 0; i <= fdmax; i++) {  
                if(i == this->sd) {
                    this->accept_new_connection(
                        &master, 
                        &fdmax
                    );
                    continue;
                }
                
                int bytes_received = recv(
                    i, 
                    this->buffer, 
                    sizeof(this->buffer), 
                    0
                );

                // TODO
                // - add 0 check and close socket

                if (bytes_received == -1) {
                    std::cerr << "Failed to receive response " << std::endl;
                    exit(EXIT_FAILURE);
                }

                std::string req(buffer, bytes_received);
                auto resp = message_callback(req);

                if (resp != ""){

                }
            }        
        }
    }

    void SetHandler(message_handler callback) {
        message_callback = callback;
    }

private:
 
    void accept_new_connection(fd_set *master, int *fdmax){
        sockaddr_in cl_addr;
        auto addrlen = sizeof(cl_addr);
        
        auto newfd = accept(
            this->sd, 
            (sockaddr *)&cl_addr, 
            (socklen_t *) &addrlen
        );

        FD_SET(this->sd, master); 
        
        if(sd > *fdmax){ 
            *fdmax = sd; 
        }
    }
};
