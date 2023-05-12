#include "network.h"
#include <cstdint>
#include <iterator>
#include <netinet/in.h>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <iostream>

// TODO:
//  - get MAX_MESSAGE_SIZE from config

#define MAX_MESSAGE_SIZE 1024

int StatusCodeFromRes(int code){
    switch (code) {
        case 0: return ERR_TIMEOUT;
        case -1: return ERR_BROKEN;
        default: return ERR_BROKEN;
    }
}

Response Receive(int sd) noexcept {
    size_t web_len = 0;

    auto res = recv(sd, (void*) &web_len, sizeof(size_t), 0);

    if (res <= 0){
        return Response{
            .err = StatusCodeFromRes(res)
        };
    }

	// TODO:
	//	- check type
	int len = ntohl(web_len);

    if (len < 0 || len > MAX_MESSAGE_SIZE){
        return Response{
            .err = ERR_BROKEN,
        };    
    }

    // Allocate a receive buffer
    char* data = new char[len]();
    
    if (data == nullptr){
        return Response{
            .err = ERR_BROKEN,
        };
    }

    res = recv(sd, data, len, 0);
    if (res <= 0){
        return Response{
            .err = StatusCodeFromRes(res)
        };
    }

    std::string message;

    message.assign(data, len);
    delete []  data;

    return Response{
        .err = ERR_OK,
        .content = message
    };
}

Error Send(int sd, std::string message) noexcept {

	// TODO:
	//	- check type
    int len = message.size();

    if (len < 0 || len > MAX_MESSAGE_SIZE){
        return MESSAGE_TOO_LONG;
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

    return ERR_OK;
}