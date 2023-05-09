#include <iostream>
#include <string>

#include <chrono>
#include <thread>

#include "lib/endpoint.cpp"


void message_callback(std::string message){
    std::cout << message << std::endl;
    std::cout << "pong" << std::endl;
}

int main() {
    auto server = new Endpoint("1050");
    auto client = new Endpoint("4040");

    server->Message(message_callback); 

    // start the serve in another thread to do the later actions
    std::thread server_thread([server](){
        server->Start();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    client->Connect("127.0.0.1", "1050");
    client->Send("ping");

    server_thread.join();
    return 0;
}
