#include <cstddef>
#include <cstdint>
#include <iterator>
#include <netinet/in.h>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <memory>
#include <variant>
#include <span>

#include "protocol.h"

// TODO: get MAX_MESSAGE_SIZE from config
#define MAX_MESSAGE_SIZE 1024

std::tuple<std::vector<uint8_t>,entity::Error> protocol::RawReceive(int sd) noexcept {
    auto web_len = 0;

    auto res = recv(sd, (void*) &web_len, sizeof(size_t), 0);

    if (res <= 0){
        return {
            std::vector<uint8_t>(), 
            entity::StatusCodeFromCSocketErrorCodes(res)
        };
    }

	auto len = ntohl(web_len);

    if (len < 0 || len > MAX_MESSAGE_SIZE){
        return {
            std::vector<uint8_t>(), 
            entity::ERR_BROKEN
        };
    }


    auto message = std::vector<uint8_t>(len);

    // Allocate a receive buffer
    message.resize(len, 0x00);

    // TODO: add while
    res = recv(sd, message.data(), len, 0);
    
    return {
        message,
        entity::StatusCodeFromCSocketErrorCodes(res)
    };
}

entity::Error protocol::RawSend(int sd, std::vector<uint8_t> message) noexcept {

    auto len = message.size();

    if (len < 0 || len > MAX_MESSAGE_SIZE){
        return entity::ERR_MESSAGE_TOO_LONG;
    }
    
    auto web_len = htonl(message.size());

    // TODO: add while
    auto res = send(sd, &web_len, sizeof(size_t), 0);


    if(res <= 0){
        return entity::StatusCodeFromCSocketErrorCodes(res);
    }
    
    res = send(sd, message.data(), len, 0);
    
    return entity::StatusCodeFromCSocketErrorCodes(res);
}