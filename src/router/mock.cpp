#include "router.h"

namespace router {

    void MockPongRouter::Disconnect(int sd){}

    std::string MockPongRouter::Handle(int sd, std::string message){
        if (message != "ping"){
            return "";
        }
        return "pong";
    }
}
