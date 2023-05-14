#include "protocol.h"
#include <span>
#include <utility>

using namespace protocol;

FunkyProtocol::FunkyProtocol(FunkyOptions *opt){
    this->username = opt->username;
}

std::pair<std::span<uint8_t>,entity::Error> FunkyProtocol::RightHandshake(int sd){
    // TODO implement the RightHandshake
    //  - send the username to the server
    //  - dh using RSA generated keys
    return {};
}


std::pair<std::span<uint8_t>,entity::Error> FunkyProtocol::LeftHandshake(int sd){
    // TODO implement the LeftHandshake
    //  - send the username to the server
    //  - dh using RSA generated keys
    return {};
}

entity::Error FunkyProtocol::Send(int sd, std::string message){
    
    if (this->sessions_key == ""){
        auto res = this->LeftHandshake(sd);
        if (res.second != entity::ERR_OK){
            return res.second;
        }
        
        // maybe always use a span   
        this->sessions_key = std::string(
            res.first.begin(), 
            res.first.end()
        );
    }

    // TODO: implement encrypted send
    //  - encrypt using session key
    //  - add hash for integreity
    //  - call raw send

    return entity::ERR_OK;
}

std::pair<std::string,entity::Error> FunkyProtocol::Receive(int sd){
    if (this->sessions_key == ""){
        auto res = this->RightHandshake(sd);
        if (res.second != entity::ERR_OK){
            return {{},res.second};
        }
        
        // maybe always use a span   
        this->sessions_key = std::string(
            res.first.begin(), 
            res.first.end()
        );
    }


    // TODO: implement encrypted receive
    //  - call raw receive
    //  - extract hash and check for integrity
    //  - decrypt using session key

    return {};
}
