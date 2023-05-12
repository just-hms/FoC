#include <string>
#include <functional>

#define BUF_LEN 1024

// general structures
typedef int error;

#define NIL                  0
#define ERR_CONN_REFUSED    -1
#define ERR_DISCONNECTED    -3
#define ERR_BROKEN          -4

typedef std::function<std::string(std::string)> handler;

struct Response{
    error err;
    std::string content;
};

Response Receive(int sd) noexcept;
error Send(int sd, std::string message) noexcept;


// Client class

class Client {
private:
    int sd;

    std::string server_ip;
    int server_port;
public:
    Client(std::string server_ip, int port);
    error Connect();
    Response Request(std::string message, uint timeout_seconds = 0);
};


// Server class
class Server {
private:
    int listener;
    int port;
    handler callback;
public:
    Server(int port);
    void Listen();
    void SetHandler(handler callback);
private:
    int acceptNewConnection(fd_set *master);
};
