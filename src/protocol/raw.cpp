#include "protocol.h"
#include <iostream>

namespace protocol{
    entity::Error RawProtocol::Send(int sd, std::string message) {
        return RawSend(
            sd, 
            std::vector<uint8_t>(message.begin(),message.end())
        );
    }

    std::tuple<std::string,entity::Error> RawProtocol::Receive(int sd) { 
        auto [res, err] = RawReceive(sd);
        return {
            std::string(res.begin(), res.end()),
            err,
        };
    }
}