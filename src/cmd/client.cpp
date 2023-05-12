#include <iostream>
#include "./../network/network.h"
#include "./../config/config.h"

using namespace std;

int main() {
    auto config = new Config();
    auto client = new Client(
        "127.0.0.1", 
        config->ServerPort
    );
    client->Connect();
    auto res = client->Request("ping");

    std::cout << res.content << std::endl;
    return 0;
}
