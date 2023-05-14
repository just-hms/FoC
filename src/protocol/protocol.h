#include <string>
#include <span>
#include "./../entity/entity.h"

std::pair<std::span<uint8_t>,entity::Error> RawReceive(int sd) noexcept;
entity::Error RawSend(int sd, std::span<uint8_t> message) noexcept;

namespace protocol {
    
    class IProtocol {
    public:
        virtual ~IProtocol() {}
        virtual entity::Error Send(int sd, std::string message) = 0;
        virtual std::pair<std::string,entity::Error> Receive(int sd) = 0;
    };

    class RawProtocol : public protocol::IProtocol{
    public:
        ~RawProtocol() {}
        virtual entity::Error Send(int sd, std::string message) {

            auto vec = std::vector<uint8_t>(
                message.begin(),
                message.end()
            );

            return RawSend(
                sd, 
                std::span<uint8_t>(vec)
            );
        }
        virtual std::pair<std::string,entity::Error> Receive(int sd) { 
            auto res = RawReceive(sd);
            return {
                std::string(res.first.begin(), res.first.end()),
                res.second,
            };
        }
    };

    struct FunkyOptions {
        std::string username;
    };

    class FunkyProtocol : public protocol::IProtocol {
    private:
        std::string sessions_key;
        std::string username;
    public:
        // TODO: edit constructor to accept cfg
        ~FunkyProtocol() {}
        FunkyProtocol(FunkyOptions *opt);

        virtual entity::Error Send(int sd, std::string message);
        virtual std::pair<std::string,entity::Error> Receive(int sd);
        
        void SetUsername(std::string username);
        std::pair<std::span<uint8_t>,entity::Error> LeftHandshake(int sd);
        std::pair<std::span<uint8_t>,entity::Error> RightHandshake(int sd);
    };
}


