#include <iostream>
#include <string>

#include <chrono>
#include <thread>

#include "lib/endpoint.cpp"


auto client1 = new Endpoint("4040");

void client_call_back(std::string message){
    client1->Send("ping");
}

void server_message_callback(std::string message){
    if (message != "ping"){
        return;
    }
    std::cout << "pong" << std::endl;
}


int main() {
    auto server = new Endpoint("5050");
    server->Message(server_message_callback); 

    // start the serve in another thread to do the later actions
    std::thread server_thread([server](){
        server->Start();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    client1->Connect("127.0.0.1", "5050");
    client1->Input(client_call_back);
    
    client1->StartInputLoop();

    server_thread.join();
    return 0;
}
