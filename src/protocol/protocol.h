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
        ~RawProtocol() {}
        virtual entity::Error Send(int sd, std::string message) { return RawSend(sd, message); }
        virtual entity::Response Receive(int sd) { return RawReceive(sd); }
    };

    struct FunkyOptions {
        std::string username;
    };

    struct HandshakeResponse {
        std::string sessionkey;
        entity::Error err;
    };

    class FunkyProtocol : public protocol::IProtocol{
    private:
        std::string sessions_key;
        std::string username;
    public:
        // TODO:
        //  - edit constructor to accept cfg
        ~FunkyProtocol() {}
        FunkyProtocol(FunkyOptions *opt);
        
        virtual entity::Error Send(int sd, std::string message);
        virtual entity::Response Receive(int sd);
        
        void SetUsername(std::string username);
        HandshakeResponse ClientHandshake(int sd);
        HandshakeResponse ServerHandshake(int sd);
    };
}


