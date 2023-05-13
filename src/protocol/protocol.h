#include <string>
#include "./../entity/entity.h"

// TODO:
//  - make this not accessible to outside
entity::Response RawReceive(int sd) noexcept;
entity::Error RawSend(int sd, std::string message) noexcept;

namespace protocol {
    
    class IProtocol {
    public:
        virtual ~IProtocol() {}
        virtual entity::Error Send(int sd, std::string message) = 0;
        virtual entity::Response Receive(int sd) = 0;
    };

    class RawProtocol : public protocol::IProtocol{
    public:
        RawProtocol(){};
        ~RawProtocol() {}
        entity::Error Send(int sd, std::string message) { return RawSend(sd, message); }
        entity::Response Receive(int sd) { return RawReceive(sd); }
    };

    class FunkyProtocol : public protocol::IProtocol{
    private:
        bool handshake = false;
    public:
        // TODO:
        //  - edit constructor to accept cfg
        FunkyProtocol(){};
        ~FunkyProtocol() {}
        entity::Error SendHandshake(int sd, std::string message);
        entity::Error ReceiveHandshake(int sd);
        entity::Error Send(int sd, std::string message);
        entity::Response Receive(int sd);
    };
}


