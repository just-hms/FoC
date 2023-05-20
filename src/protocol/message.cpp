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
#include <errno.h>
#include "protocol.h"
#include <span>

// TODO: get MAX_MESSAGE_SIZE from config
constexpr int MAX_MESSAGE_SIZE =    4096;
constexpr int MAX_CHUNK_SIZE =      1024;

std::tuple<std::vector<uint8_t>,entity::Error> protocol::RawReceive(int sd) noexcept {
    
    //receive actual size
    auto web_len = 0;
    auto res = recv(sd, (void*) &web_len, sizeof(size_t), 0);
    if(res <= 0) {
        return {
            std::vector<uint8_t>(), 
            entity::StatusCodeFromCSocketErrorCodes(res)
        };
    }

	auto len = ntohl(web_len);
    if(len < 0 || len > MAX_MESSAGE_SIZE) {
        return {
            std::vector<uint8_t>(), 
            entity::ERR_BROKEN
        };
    }

    //receive message
    int bytes_received = 0, tmp_len;
    std::vector<uint8_t>message, buffer(MAX_CHUNK_SIZE);
    while(bytes_received < len) {
        buffer.clear();
        res = recv(sd, (void*) &web_len, sizeof(size_t), 0);
        if(res <= 0) return { std::vector<uint8_t>(), entity::StatusCodeFromCSocketErrorCodes(res)};
        tmp_len = ntohl(web_len);

        buffer.resize(tmp_len);
        res = recv(sd, buffer.data(), tmp_len, 0);
        if(res <= 0) return { std::vector<uint8_t>(), entity::StatusCodeFromCSocketErrorCodes(res)};
        bytes_received += res;

        message.insert(message.end(), buffer.begin(), buffer.end());
    }

    return {
        message,
        entity::StatusCodeFromCSocketErrorCodes(res)
    };
}

entity::Error protocol::RawSend(int sd, std::vector<uint8_t> message) noexcept {

    //check if the message can be sent
    auto len = message.size();
    if(len < 0 || len > MAX_MESSAGE_SIZE) {
        return entity::ERR_MESSAGE_TOO_LONG;
    }

    //send actual size
    auto web_len = htonl(message.size());
    auto res = send(sd, &web_len, sizeof(size_t), 0);
    if(res <= 0){
        return entity::StatusCodeFromCSocketErrorCodes(res);
    }
    
    //send message
    int bytes_sent = 0, tmp_len;
    while(bytes_sent < len) {
        tmp_len = (len - bytes_sent > MAX_CHUNK_SIZE) ? MAX_CHUNK_SIZE : len - bytes_sent;
        web_len = htonl(tmp_len);
        res = send(sd, &web_len, sizeof(size_t), 0);
        if(res <= 0) return entity::StatusCodeFromCSocketErrorCodes(res);

        res = send(sd, message.data()+bytes_sent, tmp_len, 0);
        if(res <= 0) return entity::StatusCodeFromCSocketErrorCodes(res);
        bytes_sent += res;
    }
    
    return entity::StatusCodeFromCSocketErrorCodes(res);
}