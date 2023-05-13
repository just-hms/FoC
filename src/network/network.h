#include <string>
#include <functional>

#include "./../protocol/protocol.h"

#define BUF_LEN 1024

// general structures

typedef std::function<std::string(int, std::string)> handler;

// Client class
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
    Client(ClientOption *opt);
    ~Client();
    entity::Error Connect();
    entity::Response Request(std::string message);
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
    protocol::IProtocol* proto;
    handler callback;
public:
    Server(ServerOption *opt) noexcept;
    ~Server();
    void Listen();
    void SetHandler(handler callback);
private:
    int acceptNewConnection(fd_set *master);
};
