#include "network.h"
#include <cstdint>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <iostream>

Response Receive(int sd){
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
        .err = NIL,
        .content = message
    };
}

error Send(int sd, std::string message) {

    size_t len = htonl(message.size());

    auto res = send(sd, &len, sizeof(size_t), 0);
    if(res == 0){    
        return ERR_DISCONNECTED;
    }
    if(res == -1){    
        return ERR_BROKEN;
    }
    
    res = send(sd, message.c_str(), message.size(), 0);
    if(res == 0){    
        return ERR_DISCONNECTED;
    }
    if(res == -1){    
        return ERR_BROKEN;
    }
    return NIL;
}