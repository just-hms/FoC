#include "test.h"

int TestRawPingPong() {
    config::Config cfg;
    protocol::RawProtocol p;
    router::MockPongRouter router;

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
        .router = &router
    };
    net::Server s(&server_opt);

    // start the server in another thread
    std::thread server_thread([&s](){
        s.Listen();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // connnect the client
    
    c.Connect();
    auto [res, err] = c.Request("ping");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // stop the server
    s.Stop();
    server_thread.join();

    // check the response
    ASSERT_EQUAL("pong", res);
    ASSERT_EQUAL(entity::ERR_OK, err);

    TEST_PASSED();
}

int TestFunkyPingPong() {
    config::Config cfg;
    protocol::FunkyProtocol p;
    router::MockPongRouter router;

    // create the client
    net::ClientOption client_opt{
        .server_ip = "127.0.0.1",
        .port = cfg.ServerPort + 1,
        .proto = &p,
        .timeout = 200,
    };
    net::Client c(&client_opt);

    // create the server
    net::ServerOption server_opt{
        .port = cfg.ServerPort + 1,
        .proto = &p,
        .router = &router
    };
    net::Server s(&server_opt);

    // start the server in another thread
    std::thread server_thread([&s](){
        s.Listen();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // connnect the client
    
    c.Connect();
    auto [res, err] = c.Request("ping");
    ASSERT_EQUAL("pong", res);

    auto [res2, err2] = c.Request("ping");
    ASSERT_EQUAL("pong", res2);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // stop the server
    s.Stop();
    server_thread.join();

    // check the response
    ASSERT_EQUAL(entity::ERR_OK, err);

    TEST_PASSED();
    TEST_PASSED();
}
