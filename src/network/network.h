#include <string>
#include <functional>

#define BUF_LEN 1024

// general structures
typedef int Error;

#define ERR_OK              +0
#define ERR_NOT_FOUND       -1
#define ERR_TIMEOUT         -2
#define ERR_BROKEN          -3
#define MESSAGE_TOO_LONG    -5

typedef std::function<std::string(std::string)> handler;

struct Response{
    Error err;
    std::string content;
};

Response Receive(int sd) noexcept;
Error Send(int sd, std::string message) noexcept;

// Client class
struct ClientOption {
    std::string server_ip;
    int port;
    
    // milliseconds
    int timeout = -1;
};

class Client {
private:
    int sd;

    std::string server_ip;
    int server_port;
public:
    Client(ClientOption *opt);
    Error Connect();
    Response Request(std::string message);
};

struct ServerOption {
    int port;
};

// Server class
class Server {
private:
    int listener;
    int port;
    handler callback;
public:
    Server(ServerOption *opt) noexcept;
    void Listen();
    void SetHandler(handler callback);
private:
    int acceptNewConnection(fd_set *master);
};
