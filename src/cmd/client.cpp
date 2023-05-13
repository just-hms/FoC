#include <iostream>
#include "./../network/network.h"
#include "./../config/config.h"

int main() {
    Config cfg{};


    ClientOption opt{
        .server_ip = "127.0.0.1",
        .port = cfg.ServerPort,
        .timeout = 200,
    };

    Client client(&opt);
    client.Connect();


    // ping pong
    auto res = client.Request("{\"username\":\"kek\", \"password\":\"kek\"");
    std::cout << "content: " << res.content << std::endl;
   
    // timeout check
    auto res2 = client.Request("kek");
    std::cout << "timed out: " << std::endl;

    return 0;
}
