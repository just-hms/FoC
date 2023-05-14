#include "protocol.h"

using namespace protocol;


FunkyProtocol::FunkyProtocol(FunkyOptions *opt){
    this->username = opt->username;
}

HandshakeResponse FunkyProtocol::RightHandshake(int sd){
    // TODO
    //  - send the username to the server
    //  - dh using RSA generated keys
    return HandshakeResponse{};
}

HandshakeResponse FunkyProtocol::LeftHandshake(int sd){
    // TODO
    //  - send the username to the server
    //  - dh using RSA generated keys
    return HandshakeResponse{};
}


entity::Error FunkyProtocol::Send(int sd, std::string message){
    
    if (this->sessions_key == ""){
        auto res = this->LeftHandshake(sd);
        if (res.err != entity::ERR_OK){
            return res.err;
        }
        this->sessions_key = res.sessionkey;
    }

    // TODO:
    //  - encrypt using session key
    //  - add hash for integreity
    //  - call raw send

    return entity::ERR_OK;
}


entity::Response FunkyProtocol::Receive(int sd){
    if (this->sessions_key== ""){
        auto res = this->RightHandshake(sd);
        if (res.err != entity::ERR_OK){
            return entity::Response{
                .err = res.err
            };
        }
        this->sessions_key = res.sessionkey;
    }
    // TODO
    //  - implement
    return entity::Response{};
}
