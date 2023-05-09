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
    auto server = new Endpoint("5050");
    auto client1 = new Endpoint("4040");
    auto client2 = new Endpoint("4041");

    server->Message(message_callback); 

    // start the serve in another thread to do the later actions
    std::thread server_thread([server](){
        server->Start();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    client1->Connect("127.0.0.1", "5050");
    client1->Send("ping1");
    
    client2->Connect("127.0.0.1", "5050");
    client2->Send("ping2");

    server_thread.join();
    return 0;
}
