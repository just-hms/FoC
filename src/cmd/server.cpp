#include <iostream>
#include <string>

#include "./../config/config.h"
#include "./../network/network.h"
#include "./../router/router.h"
#include "./../protocol/protocol.h"
#include "./../repo/repo.h"


int main() {
    config::Config cfg;

    protocol::FunkyOptions fOpt{
        .name = "server",
        .dataPath = "./data/keys/",
        .secret = cfg.Secret,
    };

    protocol::FunkyProtocol p(&fOpt);
    repo::BankRepo repo("./data/", cfg.Secret, cfg.HistoryLen);
    router::Router router(&repo);
    
    net::ServerOption opt{
        .port = cfg.ServerPort,
        .proto = &p,
        .router = &router,
    };

    net::Server server(&opt);

    server.Listen();

    return 0;
}
