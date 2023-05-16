#include "router.h"

void router::MockPongRouter::Disconnect(int sd){}

std::string router::Router::Handle(int sd, std::string message){
    if (message != "ping"){
        return "";
    }
    return "pong";
}
