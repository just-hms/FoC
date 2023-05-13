#include "protocol.h"

using namespace protocol;

entity::Error FunkyProtocol::ReceiveHandshake(int sd){
    // TODO
    //  - implement
    return entity::ERR_OK;
}

entity::Error FunkyProtocol::SendHandshake(int sd, std::string message){
    // TODO:
    //  - implement handshake
    return entity::ERR_OK;
}


entity::Error FunkyProtocol::Send(int sd, std::string message){
    if (this->handshake == false){
        auto err = this->SendHandshake(sd, message);
        if (err == entity::ERR_OK){
            this->handshake = true;
        }
        return err;
    }

    // TODO:
    //  - encrypt using sym algorithm
    //  - add hash
    //  - raw send

    return entity::ERR_OK;

}


entity::Response FunkyProtocol::Receive(int sd){
    // TODO
    //  - implement
    return entity::Response{};
}
