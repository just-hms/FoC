#include "protocol.h"
#include <iostream>

entity::Error protocol::RawProtocol::Send(int sd, std::string message) {
    return protocol::RawSend(
        sd, 
        std::vector<uint8_t>(message.begin(),message.end())
    );
}

std::tuple<std::string,entity::Error> protocol::RawProtocol::Receive(int sd) { 
    auto [res, err] = protocol::RawReceive(sd);
    return {
        std::string(res.begin(), res.end()),
        err,
    };
}