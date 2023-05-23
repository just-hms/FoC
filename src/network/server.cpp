#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>

#include "network.h"

// Server constructor accpet a set of options to build the server object
//
// you must specicy a port and a protocol to use
net::Server::Server(ServerOption *opt) noexcept{
    if(opt->proto == nullptr){
        std::cerr << "You must specify a protocol " << std::endl;
        exit(EXIT_FAILURE);
    }

    if(opt->router == nullptr){
        std::cerr << "You must specify a router " << std::endl;
        exit(EXIT_FAILURE);
    }

    this->port = opt->port;
    this->proto = opt->proto;
    this->router = opt->router;

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

// Listen handles the incoming connection and messages
//
// it uses the specified callbacks to handle the received messages
void net::Server::Listen() noexcept{
    
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

    while(!this->stop){
        
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

            // add a connection in case the full sd is the listener
            if(fd == this->listener) {
                auto sd = this->acceptNewConnection(&master);
                if(sd > fdmax){ 
                    fdmax = sd; 
                }
                continue;
            }

            // in case of a message call the proto receive
            auto [res, err] = this->proto->Receive(fd);

            // in case of an error close the socket
            if (err == entity::ERR_TIMEOUT || err == entity::ERR_BROKEN) {
                this->disconnect(&master, fd);
                continue;
            }

            // call the message callback
            auto resp = this->router->Handle(
                fd, 
                std::string(res.begin(),res.end())
            );

            // if there is no response don't respond
            if (resp == "") continue;

            // otherwise send the response

            err = this->proto->Send(fd,resp);

            if (err == entity::ERR_BROKEN || err == entity::ERR_TIMEOUT) {
                this->disconnect(&master, fd);
                continue;
            }
        }        
    }
}

// acceptNewConnection add the socket to the list of available sockets
int net::Server::acceptNewConnection(fd_set *master) noexcept {
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

// disconnect handle the disconnection process
void net::Server::disconnect(fd_set *master, int sd) noexcept {
    // remove the socket from the list and close it    
    FD_CLR(sd, master);
    close(sd);

    // call the disconnected callback to make the application 
    // know that the client disconnnected
    this->router->Disconnect(sd);
    this->proto->Disconnect(sd);               
}

void net::Server::Stop() noexcept{
    this->stop = true;
}

// ~Server is the server distructor
net::Server::~Server() noexcept {;}


