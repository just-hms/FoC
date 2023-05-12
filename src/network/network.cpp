#include "network.h"
#include <cstdint>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <iostream>

// TODO:
//  - get MAX_MESSAGE_SIZE from config

#define MAX_MESSAGE_SIZE 1024

Response Receive(int sd) noexcept {
    size_t len = 0;
    auto res = recv(sd, (void*) &len, sizeof(size_t), 0);

    if(res == 0){ 
        return Response{
            .err = ERR_DISCONNECTED,
        };   
    }
    if(res == -1){
        return Response{
            .err = ERR_BROKEN,
        };    
    }

    if (len < 0 || len > MAX_MESSAGE_SIZE){
        return Response{
            .err = ERR_BROKEN,
        };    
    }

    // Allocate a receive buffer
    char* data = new char[len]();
    res = recv(sd, data, len, 0);

    if(res == 0){ 
        return Response{
            .err = ERR_DISCONNECTED,
        };
    }

    if(res == -1){
        return Response{
            .err = ERR_BROKEN,
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

error Send(int sd, std::string message) noexcept {

    int len = message.length();

    if (len < 0 || len > MAX_MESSAGE_SIZE){
        return MESSAGE_TOO_LONG;
    }

    size_t web_len = htonl(message.size());

    auto res = send(sd, &web_len, sizeof(size_t), 0);
    if(res == 0){    
        return ERR_DISCONNECTED;
    }
    if(res == -1){    
        return ERR_BROKEN;
    }
    
    res = send(sd, message.c_str(), len, 0);
    if(res == 0){    
        return ERR_DISCONNECTED;
    }
    if(res == -1){    
        return ERR_BROKEN;
    }
    return ERR_OK;
}