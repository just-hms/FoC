#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>

#include "network.h"

Server::Server(ServerOption *opt) noexcept{
    this->port = opt->port;

    if(opt->proto == nullptr){
        protocol::RawProtocol p;
        opt->proto = &p;
    }

    // create a non blocking socket
    this->listener = socket(
        AF_INET, 
        SOCK_STREAM,
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

void Server::Listen(){
    
    std::cout << "server starting at port :" << this->port << std::endl;
    fd_set master, read_fds;
    
    timeval timeout{
        .tv_sec = 1,
        .tv_usec = 0
    };
         
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
            &timeout
        );

        for(int fd = 0; fd <= fdmax; fd++) {  
            
            if(!FD_ISSET(fd, &read_fds))
                continue;

            if(fd == this->listener) {
                auto sd = this->acceptNewConnection(&master);
                if(sd > fdmax){ 
                    fdmax = sd; 
                }
                continue;
            }
            
            auto res = this->proto->Receive(fd);

            if (res.err == entity::ERR_TIMEOUT || res.err == entity::ERR_BROKEN) {
                FD_CLR(fd, &master);
                close(fd);
                continue;
            }
            if (this->callback == nullptr){
                std::cout << "Warning: no message handler specified" << std::endl;               
                continue;
            }
            auto resp = callback(fd, res.content);

            if (resp == "") continue;

            auto err = this->proto->Send(fd,resp);

            if (err == entity::ERR_BROKEN) {
                FD_CLR(fd, &master);
                close(fd);
                continue;
            }
        }        
    }
}

void Server::SetHandler(handler callback) {
    this->callback = callback;
}

int Server::acceptNewConnection(fd_set *master){
    sockaddr_in cl_addr;
    auto addrlen = sizeof(cl_addr);
    
    auto newsd = accept(
        this->listener, 
        (sockaddr *)&cl_addr, 
        (socklen_t *) &addrlen
    );

    FD_SET(newsd, master); 
    return newsd;
}

Server::~Server(){;}
