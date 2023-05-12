#include <string>
#include <functional>

#define BUF_LEN 1024

// general structures
typedef int error;

#define ERR_OK              +0
#define ERR_NOT_FOUND       -1
#define ERR_DISCONNECTED    -2
#define ERR_BROKEN          -3
#define ERR_TIMEOUT         -4
#define MESSAGE_TOO_LONG    -5

typedef std::function<std::string(std::string)> handler;

struct Response{
    error err;
    std::string content;
};

Response Receive(int sd) noexcept;
error Send(int sd, std::string message) noexcept;

// Client class
struct ClientOption {
    std::string server_ip;
    int port;
    int timeout;
};

class Client {
private:
    int sd;

    std::string server_ip;
    int server_port;
public:
    Client(ClientOption *opt);
    error Connect();
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
