#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <span>
#include <tuple>

#include "./../entity/entity.h"
#include "./../security/security.h"


namespace protocol {

    std::tuple<std::vector<uint8_t>,entity::Error> RawReceive(int sd) noexcept;
    entity::Error RawSend(int sd, std::vector<uint8_t> message) noexcept;
    
    // IProtocol is an interface that represent a protocol
    class IProtocol {
    public:
        virtual ~IProtocol() {}
        virtual entity::Error Send(int sd, std::string message) = 0;
        virtual std::tuple<std::string,entity::Error> Receive(int sd) = 0;
    };

    // RawProtocol implements the IProtocol sending raw data
    class RawProtocol : public protocol::IProtocol{
    public:
        ~RawProtocol() {}
        virtual entity::Error Send(int sd, std::string message);
        virtual std::tuple<std::string,entity::Error> Receive(int sd);
    };

    struct FunkyOptions {
        std::string username;
    };

    // FunkyProtocol implements the IProtocol sending encrypted data
    class FunkyProtocol : public protocol::IProtocol {
    private:

        // TODO put all of this in a map
        bool InSession;
        std::string username;
        
        sec::SymCrypt sym;
        sec::Hmac mac;
        sec::AsymCrypt asy;
        
        // lazy handshakes
        //  - this function are called silently inside the Send and Receive
        //  - the handshake starts if no sessions_key is currently available
        std::tuple<sec::sessionKey,entity::Error> LeftHandshake(int sd);
        std::tuple<sec::sessionKey,entity::Error> RightHandshake(int sd);
    public:
        // TODO: edit constructor to accept cfg
        ~FunkyProtocol() {}
        FunkyProtocol(FunkyOptions *opt);

        // Send and Receive implementations
        virtual entity::Error Send(int sd, std::string message);
        virtual std::tuple<std::string,entity::Error> Receive(int sd);

        void SetUsername(std::string username);
    };
}

#endif
