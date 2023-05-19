#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <memory>
#include <string>
#include <span>
#include <tuple>
#include <unordered_map>

#include "./../entity/entity.h"
#include "./../security/security.h"
#include "./../network/network.h"
#include "time.h"


namespace protocol {

    std::tuple<std::vector<uint8_t>,entity::Error> RawReceive(int sd) noexcept;
    entity::Error RawSend(int sd, std::vector<uint8_t> message) noexcept;
    
    // RawProtocol implements the IProtocol sending raw data
    class RawProtocol : public net::IProtocol{
    public:
        ~RawProtocol() {}
        virtual entity::Error Send(int sd, std::string message);
        virtual std::tuple<std::string,entity::Error> Receive(int sd);
    };

    struct FunkySecuritySuite {
        // std::string username;
        std::shared_ptr<sec::SymCrypt> sym;
        std::shared_ptr<sec::Hmac> mac;
    };

    // FunkyProtocol implements the IProtocol sending encrypted data
    class FunkyProtocol : public net::IProtocol {
    private:    

        std::unordered_map<int, FunkySecuritySuite> sessions;
        
        // lazy handshakes
        //  - this function are called silently inside the Send and Receive
        //  - the handshake starts if no sessions_key is currently available
        std::tuple<FunkySecuritySuite,entity::Error> LeftHandshake(int sd);
        std::tuple<FunkySecuritySuite,entity::Error> RightHandshake(int sd);

    public:
        // TODO: edit constructor to accept cfg
        ~FunkyProtocol() {}
        FunkyProtocol();

        // Send and Receive implementations
        virtual entity::Error Send(int sd, std::string message);
        virtual std::tuple<std::string,entity::Error> Receive(int sd);

        void SetUsername(std::string username);
    };
}

#endif
