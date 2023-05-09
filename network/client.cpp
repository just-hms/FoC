#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <functional>

#include "network.h"

using namespace std;

Client::Client(string server_ip, int port) {
    sd = socket(
        AF_INET, 
        SOCK_STREAM ,
        0
    );
    
    if (sd == -1) {
        cerr << "Failed to create socket " << endl;
        exit(EXIT_FAILURE);
    }
    
    this->server_ip = server_ip;
    this->server_port = port;
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
    return NIL;
}

Response Client::Request(string message, uint timeout_seconds) {
        
    auto res = Send(this->sd, message);

    if (res == -1) {
        cerr << "Failed to send message " << endl;
        exit(EXIT_FAILURE);
    }

    return Receive(this->sd);
}
