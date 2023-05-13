#include <cerrno>
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>

#include "network.h"


// TODO
//  - find where to do the handshake

Client::Client(ClientOption* opt) {
    this->sd = socket(
        AF_INET, 
        SOCK_STREAM ,
        0
    );

    // if no protocol is specified used the default
    if(opt->proto == nullptr){
        protocol::RawProtocol p;
        opt->proto = &p;
    }

    if (this->sd == -1) {
        std::cerr << "Failed to create socket " << std::endl;
        exit(EXIT_FAILURE);
    }


    if (opt->timeout > 0) {
        
        timeval recive_timeout = {
            .tv_sec = 0,
            .tv_usec = opt->timeout * 1000
        };

        auto res = setsockopt(
            this->sd, 
            SOL_SOCKET, 
            SO_RCVTIMEO, 
            &recive_timeout, 
            sizeof(timeval)
        );

        if (res == -1){
            std::cerr << "Failed to create socket " << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    this->server_ip = opt->server_ip;
    this->server_port = opt->port;
}

entity::Error Client::Connect() {
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(this->server_ip.c_str());
    server_address.sin_port = htons(this->server_port);

    auto res = connect(
        this->sd, 
        (struct sockaddr*) &server_address, 
        sizeof(server_address)
    );

    if (res == -1) {
        return entity::ERR_BROKEN;
    }
    return entity::ERR_OK;
}

entity::Response Client::Request(std::string message) {
    auto err = this->proto->Send(this->sd, message);

    if (err != entity::ERR_OK) {
        return entity::Response{
            .err = err
        };
    }
    
    return this->proto->Receive(this->sd);
}

Client::~Client() {;}
