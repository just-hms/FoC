#ifndef CONFIG_H
#define CONFIG_H

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>

#include <fstream>
#include <iostream>

namespace config {
class Config {
   public:
    int ServerPort;
    std::string Secret;
    int HistoryLen;
    Config();
};
}  // namespace config

#endif  // CONFIG_H
