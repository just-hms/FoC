#include <iostream>
#include <string>

#include "./../config/config.h"
#include "./../network/network.h"
#include "./../protocol/protocol.h"
#include "./../repo/repo.h"
#include "./../router/router.h"

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

    if (server.Bind() != 0) {
        std::cerr << "Failed to bind to port " << cfg.ServerPort << std::endl;
        return 1;
    }

    std::cout << "server starting at port :" << cfg.ServerPort << std::endl;
    server.Listen();

    return 0;
}
