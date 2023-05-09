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
    int listener;
    int port;
    message_handler message_callback;
    char buffer[BUF_LEN];

public:
    
    Server(int port) {
        // create a non blocking socket
        this->listener = socket(
            AF_INET, 
            SOCK_STREAM | SOCK_NONBLOCK,
            0
        );
        if (this->listener == -1) {
            std::cerr << "Failed to create socket " << std::endl;
            exit(EXIT_FAILURE);
        }

        // bind the specified port to the socket
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        auto res = bind(
            this->listener, 
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
         
        listen(this->listener, 10);

        FD_ZERO(&master);
        FD_ZERO(&read_fds);

        FD_SET(this->listener, &master);

        auto fdmax = listener;          

        while(true){
            read_fds = master;     
        
            select(
                fdmax + 1,
                &read_fds, 
                NULL, 
                NULL, 
                NULL
            );

            for(int fd = 0; fd <= fdmax; fd++) {  
                if(fd == this->listener) {
                    auto sd = this->accept_new_connection(&master);
                    if(sd > fdmax){ 
                        fdmax = sd; 
                    }
                    continue;
                }
                
                // receving a message
                int bytes_received = recv(
                    fd, 
                    this->buffer, 
                    sizeof(this->buffer), 
                    0
                );

                if (bytes_received == 0) {
                    FD_CLR(fd, &master);
                    close(fd);
                }

                if (bytes_received == -1) {
                    std::cerr << "Failed to receive response " << std::endl;
                    exit(EXIT_FAILURE);
                }

                std::string req(buffer, bytes_received);
                auto resp = message_callback(req);

                if (resp != "")continue;
            }        
        }
    }

    void SetHandler(message_handler callback) {
        message_callback = callback;
    }

private:
 
    int accept_new_connection(fd_set *master){
        sockaddr_in cl_addr;
        auto addrlen = sizeof(cl_addr);
        
        auto newsd = accept(
            this->listener, 
            (sockaddr *)&cl_addr, 
            (socklen_t *) &addrlen
        );

        FD_SET(this->listener, master); 
        return newsd;
    }
};
