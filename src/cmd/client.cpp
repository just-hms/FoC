#include <iostream>
#include "./../network/network.h"
#include "./../config/config.h"

using namespace std;

int main() {
    Config cfg{};
    
    ClientOption opt{
        .server_ip = "127.0.0.1",
        .port = cfg.ServerPort,
    };

    Client client(&opt);
    client.Connect();
    auto res = client.Request("ping");
    
    std::cout << "status :" << res.err << std::endl;
    std::cout << "content :" << res.content << std::endl;
    return 0;
}
