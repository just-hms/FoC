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
    
    this->server_ip = opt->server_ip;
    this->server_port = opt->port;
}

error Client::Connect() {
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

    if (res == -1) {
        std::cerr << "Failed to send message " << std::endl;
        exit(EXIT_FAILURE);
    }

    return Receive(this->sd);
}
