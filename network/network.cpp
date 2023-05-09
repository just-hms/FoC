#include "network.h"
#include <vector>

Response Receive(int sd){
    size_t len = 0;

    auto res = recv(sd, (void*)&len, sizeof(size_t), 0);

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
    std::vector<uint8_t> rcvBuf;    
    rcvBuf.resize(len,0x00);

    res = recv(sd,&(rcvBuf[0]),len,0);

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

    message.assign((rcvBuf[0]),rcvBuf.size());
    
    return Response{
        .err = NIL,
        .content = message
    };
}

error Send(int sd, std::string message){

    size_t len = htonl(message.size());

    auto res = send(sd,&len ,sizeof(size_t) ,0);
    if(res == 0){    
        return ERR_DISCONNECTED;
    }
    if(res == -1){    
        return ERR_BROKEN;
    }
    
    res = send(sd,message.c_str(),message.size(), 0);
    if(res == 0){    
        return ERR_DISCONNECTED;
    }
    if(res == -1){    
        return ERR_BROKEN;
    }
    return NIL;
}