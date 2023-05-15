#include "config.h"

#define SERVER_PORT "SERVER_PORT"
 

using namespace config;

// Config constructor reads the config from config.json in the current path
Config::Config() noexcept{
    
    std::ifstream ifs("config.json");
    Json::Reader reader;
    Json::Value obj;

    reader.parse(ifs, obj);

    // get the server port (5050 as default)

    this->ServerPort = (obj[SERVER_PORT].empty()) ? 
        5050 : 
        obj[SERVER_PORT].asInt();
}