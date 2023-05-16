#include "test.h"

int TestRawPingPong() {
    config::Config cfg{};
    protocol::RawProtocol p;

    // create the client
    net::ClientOption client_opt{
        .server_ip = "127.0.0.1",
        .port = cfg.ServerPort,
        .proto = &p,
        .timeout = 200,
    };
    net::Client c(&client_opt);

    // create the server
    net::ServerOption server_opt{
        .port = cfg.ServerPort,
        .proto = &p,
    };
    net::Server s(&server_opt);

    s.SetRequestHandler([](int sd, std::string message)->std::string {
        std::cout << "[server] received : "  << message << std::endl; 
        return (message == "ping") ?
            "pong":
            "not pong";
    });

    // start the server in another thread
    std::thread server_thread([&s](){
        s.Listen();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // connnect the client
    
    c.Connect();
    auto res = c.Request("ping");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // stop the server
    s.Stop();
    server_thread.join();

    // check the response
    ASSERT("pong", res.first);
    ASSERT(entity::ERR_OK, res.second);

    TEST_PASSED();
}

int TestFunkyPingPong() {
    // TODO
    //  - write this
    TEST_PASSED();
}
