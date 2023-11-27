#include "./../config/config.h"
#include "./../entity/entity.h"
#include "./../network/network.h"
#include "./../protocol/protocol.h"
#include "./../router/router.h"
#include "./../security/security.h"
#include "./../test/test.h"

int TestRawPingPong() {
    auto DATA_PATH = std::string("/tmp/raw-ping-pong/");
    std::system(("mkdir -p " + DATA_PATH).c_str());
    defer { std::system(("rm -rf " + DATA_PATH).c_str()); };

    config::Config cfg;
    protocol::RawProtocol p;
    router::MockPongRouter router;

    // create the server
    net::ServerOption server_opt{
        .port = cfg.ServerPort, .proto = &p, .router = &router};
    net::Server s(&server_opt);

    while (s.Bind(cfg.ServerPort)) {
        cfg.ServerPort++;
    }

    // create the client
    net::ClientOption client_opt{
        .server_ip = "127.0.0.1",
        .port = cfg.ServerPort,
        .proto = &p,
        .timeout = 200,
    };
    net::Client c(&client_opt);

    // start the server in another thread
    std::thread server_thread([&s]() { s.Listen(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // connnect the client

    auto [res, err] = c.Request("ping");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // stop the server
    s.Stop();
    server_thread.join();

    // check the response
    ASSERT_EQUAL("pong", res);
    ASSERT_EQUAL(entity::ERR_OK, err);

    return 0;
}

int TestFunkyPingPong() {
    auto DATA_PATH = std::string("/tmp/funky-ping-pong/");
    std::system(("mkdir -p " + DATA_PATH).c_str());
    defer { std::system(("rm -rf " + DATA_PATH).c_str()); };

    config::Config cfg;

    // RSA GENERATION
    auto err = sec::generateRSAkeys(DATA_PATH + "server", cfg.Secret, 4096);
    ASSERT_FALSE(err < 0);

    err = sec::generateRSAkeys(DATA_PATH + "client", cfg.Secret, 4096);
    ASSERT_FALSE(err < 0);

    protocol::FunkyOptions clientFOpt{
        .name = "client",
        .peerName = "server",
        .dataPath = DATA_PATH,
        .secret = cfg.Secret,
    };

    protocol::FunkyProtocol clientP(&clientFOpt);
    router::MockPongRouter router;

    // create the server
    protocol::FunkyOptions serverFOpt{
        .name = "server",
        .dataPath = DATA_PATH,
        .secret = cfg.Secret,
    };

    protocol::FunkyProtocol serverP(&serverFOpt);

    net::ServerOption server_opt{.proto = &serverP, .router = &router};
    net::Server s(&server_opt);

    while (s.Bind(cfg.ServerPort)) {
        cfg.ServerPort++;
    }

    // create the client
    net::ClientOption client_opt{
        .server_ip = "127.0.0.1",
        .port = cfg.ServerPort,
        .proto = &clientP,
        .timeout = 200,
    };
    net::Client c(&client_opt);

    // start the server in another thread
    std::thread server_thread([&s]() { s.Listen(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // connnect the client

    auto [res, err2] = c.Request("ping");
    ASSERT_EQUAL("pong", res);

    auto [res2, err3] = c.Request("ping");
    ASSERT_EQUAL("pong", res2);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // stop the server
    s.Stop();
    server_thread.join();

    // check the response
    ASSERT_EQUAL(entity::ERR_OK, err);

    return 0;
}
