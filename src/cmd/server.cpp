#include <iostream>
#include <string>

#include "./../config/config.h"
#include "./../network/network.h"

std::string messageHandler(std::string message){
    std::cout << "message received: " << message  << std::endl;
    if (message != "ping"){
        return "";
    }
    return "pong";
}

int main() {
    Config cfg{};

    ServerOption opt{
        .port = cfg.ServerPort,
    };

    Server server(&opt);
    server.SetHandler(messageHandler); 
    server.Listen();
    return 0;
}
