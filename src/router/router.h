#include "./../entity/entity.h"
#include "./../repo/repo.h"
#include <string>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/value.h>
#include <unordered_map>
#include <iostream>
#include <memory>


// TODO: maybe make the router an object
//  - create a map of handlers

namespace router {

    typedef int reqstatus;
    constexpr reqstatus STATUS_OK = 200;
    constexpr reqstatus STATUS_BAD_REQUEST = 400;
    constexpr reqstatus STATUS_NOT_FOUND = 404;
    constexpr reqstatus STATUS_UNAUTHORIZED = 401;

    struct Context{
        std::shared_ptr<entity::User> user;
        Json::Value req;
        int connectionID;
    };

    std::string Handle(int sd, std::string message);
    void Disconnect(int sd);
}