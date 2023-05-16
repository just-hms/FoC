#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <functional>

#include "./../protocol/protocol.h"

// general structures

namespace net {

    typedef std::function<std::string(int, std::string)> RequestHandler;
    typedef std::function<void(int)> DisconnectionHandler;

    struct ClientOption {
        std::string server_ip;
        int port;
        protocol::IProtocol* proto;

        // milliseconds
        int timeout = -1;
    };

    class Client {
    private:
        int sd;
        std::string server_ip;
        int server_port;
        protocol::IProtocol* proto = nullptr;

    public:
        Client(ClientOption *opt) noexcept;
        ~Client() noexcept;
        entity::Error Connect() noexcept;
        std::tuple<std::string,entity::Error> Request(std::string message) noexcept;
    };

    struct ServerOption {
        int port;
        protocol::IProtocol* proto = nullptr;
    };

    // Server class
    class Server {
    private:
        int listener;
        int port;
        bool stop = false;
        protocol::IProtocol* proto;
        RequestHandler message_callback;
        DisconnectionHandler disconnection_callback;
    public:
        Server(ServerOption *opt) noexcept;
        ~Server() noexcept;
        void Listen() noexcept;
        void Stop() noexcept;
        void SetRequestHandler(RequestHandler callback) noexcept;
        void SetDisconnectionHandler(DisconnectionHandler callback) noexcept;
    private:
        int acceptNewConnection(fd_set *master) noexcept;
        void disconnect(fd_set *master, int sd) noexcept;
    };
}

#endif