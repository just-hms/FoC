#include <cstdint>
#include <iterator>
#include <netinet/in.h>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <iostream>

#include "protocol.h"

#define MAX_MESSAGE_SIZE 1024

// TODO:
//  - get MAX_MESSAGE_SIZE from config

#define MAX_MESSAGE_SIZE 1024

int StatusCodeFromRes(int code){
    switch (code) {
        case 0:     return entity::ERR_TIMEOUT;
        case -1:    return entity::ERR_BROKEN;
        default:    return entity::ERR_BROKEN;
    }
}

entity::Response RawReceive(int sd) noexcept {
    size_t web_len = 0;

    auto res = recv(sd, (void*) &web_len, sizeof(size_t), 0);

    if (res <= 0){
        return entity::Response{
            .err = StatusCodeFromRes(res)
        };
    }

	// TODO:
	//	- check type
	int len = ntohl(web_len);

    if (len < 0 || len > MAX_MESSAGE_SIZE){
        return entity::Response{
            .err = entity::ERR_BROKEN,
        };    
    }

    // Allocate a receive buffer
    char* data = new char[len]();
    
    if (data == nullptr){
        return entity::Response{
            .err = entity::ERR_BROKEN,
        };
    }

    res = recv(sd, data, len, 0);
    if (res <= 0){
        return entity::Response{
            .err = StatusCodeFromRes(res)
        };
    }

    std::string message;

    message.assign(data, len);
    delete []  data;

    return entity::Response{
        .err = entity::ERR_OK,
        .content = message
    };
}

entity::Error RawSend(int sd, std::string message) noexcept {

	// TODO:
	//	- check type
    int len = message.size();

    if (len < 0 || len > MAX_MESSAGE_SIZE){
        return entity::ERR_MESSAGE_TOO_LONG;
    }

    size_t web_len = htonl(message.size());

    auto res = send(sd, &web_len, sizeof(size_t), 0);

    if(res <= 0){
        return StatusCodeFromRes(res);
    }
    
    res = send(sd, message.c_str(), len, 0);
    
	if(res <= 0){
        return StatusCodeFromRes(res);
    }

    return entity::ERR_OK;
}