#include "config.h"

#define SERVER_PORT "SERVER_PORT"
 

using namespace config;

// Config constructor reads the config from config.json in the current path
Config::Config() noexcept{
    
    std::ifstream ifs("config.json");
    Json::Reader reader;
    Json::Value obj;

    reader.parse(ifs, obj);

    auto a = obj.get("kek", "");

    // get the server port (5050 as default)
    this->ServerPort = obj.get(
        SERVER_PORT, 
        5050
    ).asInt();
}