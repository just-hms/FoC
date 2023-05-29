#include <iostream>
#include <string>

#include "./../config/config.h"
#include "./../network/network.h"
#include "./../router/router.h"
#include "./../protocol/protocol.h"
#include "./../repo/repo.h"


constexpr const char * FUNKY_PATH =  "./data/";

int main() {
    config::Config cfg;


    protocol::FunkyOptions fOpt{
        .name = "server",
        .dataPath = "./data/",
        .secret = cfg.Secret,
    };

    protocol::FunkyProtocol p(&fOpt);
    repo::MockBankRepo repo;
    router::Router router(&repo);
    
    net::ServerOption opt{
        .port = cfg.ServerPort,
        .proto = &p,
        .router = &router,
    };

    net::Server server(&opt);

    server.Listen();

    std::cout<<"SERVER STARTED"<<std::endl;

    return 0;
}
