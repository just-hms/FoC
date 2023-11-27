#ifndef NETWORK_H
#define NETWORK_H

#include <functional>
#include <string>

#include "./../entity/entity.h"

// general structures

namespace net {

typedef std::function<std::string(int, std::string)> RequestHandler;
typedef std::function<void(int)> DisconnectionHandler;

// IProtocol is an interface that represent a protocol
class IProtocol {
   public:
    virtual ~IProtocol() {}
    virtual entity::Error Send(int sd, std::string message) = 0;
    virtual std::tuple<std::string, entity::Error> Receive(int sd) = 0;
    virtual void Disconnect(int sd) = 0;
};

struct ClientOption {
    std::string server_ip;
    int port;
    IProtocol *proto;

    // milliseconds
    int timeout = -1;
};

class Client {
   private:
    int sd;
    bool connected = false;
    std::string server_ip;
    int server_port, timeout;
    IProtocol *proto = nullptr;
    entity::Error _connect() noexcept;

   public:
    Client(ClientOption *opt) noexcept;
    ~Client() noexcept;
    std::tuple<std::string, entity::Error> Request(
        std::string message) noexcept;
};

class IRouter {
   public:
    virtual std::string Handle(int sd, std::string message) = 0;
    virtual void Disconnect(int sd) = 0;
};

struct ServerOption {
    int port;
    IProtocol *proto = nullptr;
    IRouter *router = nullptr;
};

// Server class
class Server {
   private:
    int listener;
    int port;
    bool stop = false;
    IProtocol *proto;
    IRouter *router;

   public:
    Server(ServerOption *opt) noexcept;
    ~Server() noexcept;
    void Listen() noexcept;
    int Bind() noexcept;
    int Bind(int port) noexcept;
    void Stop() noexcept;

   private:
    int acceptNewConnection(fd_set *master) noexcept;
    void disconnect(fd_set *master, int sd) noexcept;
};
}  // namespace net

#endif