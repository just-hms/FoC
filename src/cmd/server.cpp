#include <iostream>
#include <string>

#include "./../config/config.h"
#include "./../network/network.h"
#include "./../router/router.h"


int main() {
    config::Config cfg{};

    protocol::RawProtocol p;
    
    net::ServerOption opt{
        .port = cfg.ServerPort,
        .proto = &p,
    };

    net::Server server(&opt);

    server.SetRequestHandler(router::Handle); 
    server.SetDisconnectionHandler(router::Disconnect); 
    server.Listen();

    return 0;
}
