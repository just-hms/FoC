#include <iostream>
#include <string>

#include "./../config/config.h"
#include "./../network/network.h"

std::string messageHandler(int sd, std::string message){
    std::cout << "message received: " << message  << std::endl;
    if (message != "ping"){
        return "";
    }
    return "pong";
}

int main() {
    Config cfg{};

    protocol::RawProtocol p;
    
    ServerOption opt{
        .port = cfg.ServerPort,
        .proto = &p,
    };

    Server server(&opt);
    server.SetHandler(messageHandler); 
    server.Listen();
    return 0;
}
