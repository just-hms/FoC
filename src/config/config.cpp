#include "config.h"

constexpr const char * SERVER_PORT =  "SERVER_PORT";
constexpr const char * SECRET =  "SECRET";

// Config constructor reads the config from config.json in the current path
config::Config::Config(){
    
    std::ifstream ifs("config.json");
    Json::Reader reader;
    Json::Value obj;

    reader.parse(ifs, obj);

    // get the server port (5050 as default)
    this->ServerPort = obj.get(
        SERVER_PORT, 
        5050
    ).asInt();

    this->Secret = obj.get(
        SECRET, 
        "secret"
    ).asString();
}