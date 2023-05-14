#include <iostream>
#include <string>

#include "./../config/config.h"
#include "./../network/network.h"
#include "./../router/router.h"


int main() {
    Config cfg{};

    protocol::RawProtocol p;
    
    ServerOption opt{
        .port = cfg.ServerPort,
        .proto = &p,
    };

    Server server(&opt);
    server.SetRequestHandler(router::Handle); 
    server.SetDisconnectionHandler(router::Disconnect); 
    server.Listen();
    return 0;
}
