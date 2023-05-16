#include "protocol.h"
#include <cstdint>
#include <span>
#include <utility>
#include <vector>

protocol::FunkyProtocol::FunkyProtocol(FunkyOptions *opt){
    this->username = opt->username;
    this->sym = sec::SymCrypt(sec::sessionKey{});
    this->asy = sec::AsymCrypt("", "", "");
}

std::tuple<sec::sessionKey,entity::Error> protocol::FunkyProtocol::RightHandshake(int sd){
    // TODO implement the RightHandshake
    //  - send the username to the server
    //  - dh using RSA generated keys
    return {};
}


std::tuple<sec::sessionKey,entity::Error> protocol::FunkyProtocol::LeftHandshake(int sd){
    // TODO implement the LeftHandshake
    //  - send the username to the server
    //  - dh using RSA generated keys
    return {};
}

entity::Error protocol::FunkyProtocol::Send(int sd, std::string message){
    
    // TODO this check is not correct
    if (!this->InSession){
        auto [res, err] = this->LeftHandshake(sd);
        if (err != entity::ERR_OK){
            return err;
        }
        
        this->sym.refresh(res);
    }

    //  encrypt using session key
    auto out = this->sym.encrypt(
        std::vector<uint8_t>(message.begin(), message.end())
    );

    // generating hash for integreity
    // TODO : use vector<uint8_t> inside Hmac
    auto mac = this->mac.MAC(std::string(out.begin(), out.end()));
    
    // add hash to the message
    out.insert(out.end(), mac.begin(), mac.end());

    // call rawsend
    return protocol::RawSend(
        sd, 
        std::vector<uint8_t>(out.begin(),out.end())
    );
}

std::tuple<std::string,entity::Error> protocol::FunkyProtocol::Receive(int sd){
    if (!this->InSession){
        auto [res, err] = this->RightHandshake(sd);
        if (err != entity::ERR_OK){
            return {
                "",
                err
            };
        }

        this->sym.refresh(res);
    }

    //  call raw receive
    auto [res, err] = protocol::RawReceive(sd);
    if (err != entity::ERR_OK){
        return {
            "",
            err
        };
    }

    //  extract hash and check for integrity

    //  decrypt using session key

    return {
        "",
        entity::ERR_BROKEN
    };
}
