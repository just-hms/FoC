#include <string>
#include <span>
#include <tuple>

#include "./../entity/entity.h"

std::tuple<std::vector<uint8_t>,entity::Error> RawReceive(int sd) noexcept;
entity::Error RawSend(int sd, std::vector<uint8_t> message) noexcept;

namespace protocol {
    
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
        std::string sessions_key;
        std::string username;
        
        // lazy handshakes
        //  - this function are called silently inside the Send and Receive
        //  - the handshake starts if no sessions_key is currently available
        std::tuple<std::span<uint8_t>,entity::Error> LeftHandshake(int sd);
        std::tuple<std::span<uint8_t>,entity::Error> RightHandshake(int sd);
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


