#include "config.h"

constexpr const char * SERVER_PORT =  "SERVER_PORT";
constexpr const char * SECRET =  "SECRET";
constexpr const char * HISTORY_LEN =  "HISTORY_LEN";


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
        "f2832923fi0232m0f2jf0329hf82g8321g76r32fr327"
    ).asString();

    this->HistoryLen = obj.get(
        HISTORY_LEN, 
        5
    ).asInt();
}