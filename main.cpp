#include <iostream>
#include <string>

#include <chrono>
#include <thread>

#include "network/network.h"

using namespace std;

int server_port = 10112;

std::string messageHandler(std::string message){
    cout << "kek";
    if (message != "ping"){
        return "";
    }
    return "pong";
}

int main() {
    auto client = new Client(
        "127.0.0.1", 
        server_port
    );
    
    auto server = new Server(server_port);
    server->SetHandler(messageHandler); 

    // start the server in another thread to do the next actions
    std::thread server_thread([server](){
        server->Listen();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    client->Connect();
    
    auto res = client->Request("ping");

    std::cout << res.content << std::endl;
    
    server_thread.join();
    return 0;
}
