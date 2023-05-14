#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "./../src/entity/entity.h"
#include "./../src/router/router.h"
#include "./../src/network/network.h"
#include "./../src/config/config.h"


int main(){
    config::Config cfg{};
    protocol::RawProtocol p;

    net::ClientOption client_opt{
        .server_ip = "127.0.0.1",
        .port = cfg.ServerPort,
        .proto = &p,
        .timeout = 200,
    };
    net::Client c(&client_opt);

    net::ServerOption server_opt{
        .port = cfg.ServerPort,
        .proto = &p,
    };
    net::Server s(&server_opt);

    s.SetRequestHandler([](int sd, std::string message)->std::string {
        std::cout << "received" << message << std::endl;

        return (message == "ping") ?
            "pong":
            "";
    });

    std::thread server_thread([&s](){
        s.Listen();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    c.Connect();
    auto res = c.Request("ping");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    s.Stop();

    server_thread.join();

    return res.first != "pong";
}