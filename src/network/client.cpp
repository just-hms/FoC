#include <cerrno>
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>

#include "network.h"

Client::Client(ClientOption* opt) {
    sd = socket(
        AF_INET, 
        SOCK_STREAM ,
        0
    );

    if (sd == -1) {
        std::cerr << "Failed to create socket " << std::endl;
        exit(EXIT_FAILURE);
    }


    if (opt->timeout > 0) {
        
        timeval recive_timeout = {
            .tv_sec = 0,
            .tv_usec = opt->timeout * 1000
        };

        auto res = setsockopt(
            sd, 
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

Error Client::Connect() {
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(this->server_ip.c_str());
    server_address.sin_port = htons(this->server_port);

    auto res = connect(
        sd, 
        (struct sockaddr*) &server_address, 
        sizeof(server_address)
    );

    if (res == -1) {
        return ERR_BROKEN;
    }
    return ERR_OK;
}

Response Client::Request(std::string message) {
        
    auto res = Send(this->sd, message);

    if (res != ERR_OK) {
        return Response{
            .err = res
        };
    }

    return Receive(this->sd);
}
