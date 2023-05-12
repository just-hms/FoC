#include <iostream>
#include <string>

#include "./../config/config.h"
#include "./../network/network.h"

using namespace std;

std::string messageHandler(std::string message){
    cout << "message received: " << message  << endl;
    if (message != "ping"){
        return "";
    }
    return "pong";
}

int main() {
    auto config = new Config();
    auto server = new Server(config->ServerPort);
    server->SetHandler(messageHandler); 
    server->Listen();
    return 0;
}
