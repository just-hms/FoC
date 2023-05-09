#include <cerrno>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

using namespace std;

#include "network.h"

Server::Server(int port){

    // create a non blocking socket
    this->listener = socket(
        AF_INET, 
        SOCK_STREAM,
        0
    );
    if (this->listener == -1) {
        cerr << "Failed to create socket " << endl;
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
        cerr << "Failed to bind to port " << port << endl;
        exit(EXIT_FAILURE);
    }
}

void Server::Listen(){
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
            
            if(!FD_ISSET(fd, &read_fds))
                continue;

            if(fd == this->listener) {
                auto sd = this->acceptNewConnection(&master);
                if(sd > fdmax){ 
                    fdmax = sd; 
                }
                continue;
            }

            auto res = Receive(fd);

            if (res.err == ERR_DISCONNECTED || res.err == ERR_BROKEN) {
                FD_CLR(fd, &master);
                close(fd);
                continue;
            }

            auto resp = callback(res.content);

            if (resp != "")continue;

            auto err = Send(fd,resp);

            if (err == ERR_BROKEN) {
                FD_CLR(fd, &master);
                close(fd);
                continue;
            }
        }        
    }
}

void Server::SetHandler(handler callback) {
    callback = callback;
}

int Server::acceptNewConnection(fd_set *master){
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