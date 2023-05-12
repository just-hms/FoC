#include "config.h"

#define SERVER_PORT "SERVER_PORT"
 
Config::Config() noexcept{
    std::ifstream ifs("config.json");
    Json::Reader reader;
    Json::Value obj;
    reader.parse(ifs, obj);
    if (obj[SERVER_PORT].empty()){
        std::cout << "must specifify a " << SERVER_PORT << std::endl;
        exit(1);
    }
    this->ServerPort = obj[SERVER_PORT].asInt();
}