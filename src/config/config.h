#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>


namespace config {
    class Config {
    public:
        int ServerPort;
        std::string Secret;
        int HistoryLen;
        Config();
    };
}