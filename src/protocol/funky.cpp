#include "protocol.h"
#include <span>
#include <utility>

protocol::FunkyProtocol::FunkyProtocol(FunkyOptions *opt){
    this->username = opt->username;
}

std::tuple<std::span<uint8_t>,entity::Error> protocol::FunkyProtocol::RightHandshake(int sd){
    // TODO implement the RightHandshake
    //  - send the username to the server
    //  - dh using RSA generated keys
    return {};
}


std::tuple<std::span<uint8_t>,entity::Error> protocol::FunkyProtocol::LeftHandshake(int sd){
    // TODO implement the LeftHandshake
    //  - send the username to the server
    //  - dh using RSA generated keys
    return {};
}

entity::Error protocol::FunkyProtocol::Send(int sd, std::string message){
    
    if (this->sessions_key == ""){
        auto [res, err] = this->LeftHandshake(sd);
        if (err != entity::ERR_OK){
            return err;
        }
        
        // maybe always use a span   
        this->sessions_key = std::string(
            res.begin(), 
            res.end()
        );
    }

    // TODO: implement encrypted send
    //  - encrypt using session key
    //  - add hash for integreity
    //  - call raw send

    return entity::ERR_OK;
}

std::tuple<std::string,entity::Error> protocol::FunkyProtocol::Receive(int sd){
    if (this->sessions_key == ""){
        auto [res, err] = this->RightHandshake(sd);
        if (err != entity::ERR_OK){
            return {
                "",
                err
            };
        }
        
        // TODO: maybe always use a span   
        this->sessions_key = std::string(
            res.begin(), 
            res.end()
        );
    }


    // TODO: implement encrypted receive
    //  - call raw receive
    //  - extract hash and check for integrity
    //  - decrypt using session key

    return {
        "",
        entity::ERR_BROKEN
    };
}
