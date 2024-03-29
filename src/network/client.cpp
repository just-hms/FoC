#include <arpa/inet.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <string>

#include "network.h"

namespace net {

// Client constructor
//
// you must a port and ip of the server to connect to and a protocol to use
Client::Client(ClientOption* opt) noexcept {
    if (opt->proto == nullptr) {
        std::cerr << "You must specify a protocol " << std::endl;
        exit(EXIT_FAILURE);
    }

    this->server_ip = opt->server_ip;
    this->server_port = opt->port;
    this->proto = opt->proto;
    this->timeout = opt->timeout;
}

// Connect tries to connect to the specified server
entity::Error Client::_connect() noexcept {
    if (connected) return entity::ERR_OK;

    this->sd = socket(AF_INET, SOCK_STREAM, 0);

    if (this->sd == -1) {
        std::cerr << "Failed to create socket " << std::endl;
        exit(EXIT_FAILURE);
    }

    if (this->timeout > 0) {
        timeval recive_timeout = {.tv_sec = 0, .tv_usec = this->timeout * 1000};

        auto res = setsockopt(this->sd, SOL_SOCKET, SO_RCVTIMEO,
                              &recive_timeout, sizeof(timeval));

        if (res == -1) {
            std::cerr << "Failed to create socket " << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    connected = true;

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(this->server_ip.c_str());
    server_address.sin_port = htons(this->server_port);

    auto res = connect(this->sd, (struct sockaddr*)&server_address,
                       sizeof(server_address));

    if (res == 0)
        return entity::StatusCodeFromCSocketErrorCodes(
            1);  // connect returns 0 on success
    else
        return entity::StatusCodeFromCSocketErrorCodes(res);
}

// Request tries to make a request to the server, returns an error in case of
// failing
std::tuple<std::string, entity::Error> Client::Request(
    std::string message) noexcept {
    auto err = this->_connect();
    err = this->proto->Send(this->sd, message);
    if (err != entity::ERR_OK) {
        this->proto->Disconnect(sd);
        this->connected = false;
        close(this->sd);
        return {"", err};
    }

    auto [res, errRec] = this->proto->Receive(this->sd);
    if (errRec != entity::ERR_OK) {
        this->connected = false;
        this->proto->Disconnect(sd);
        close(this->sd);
    }

    return {res, errRec};
}

// ~Client is the client distructor
Client::~Client() noexcept { ; }

}  // namespace net