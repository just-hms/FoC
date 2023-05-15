#include "protocol.h"
#include <iostream>

using namespace protocol;

entity::Error RawProtocol::Send(int sd, std::string message) {
    return RawSend(
        sd, 
        std::vector<uint8_t>(message.begin(),message.end())
    );
}

std::pair<std::string,entity::Error> RawProtocol::Receive(int sd) { 
    auto res = RawReceive(sd);
    return {
        std::string(res.first.begin(), res.first.end()),
        res.second,
    };
}