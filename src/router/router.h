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
    #define STATUS_OK              reqstatus(200)
    #define STATUS_BAD_REQUEST     reqstatus(400)
    #define STATUS_NOT_FOUND       reqstatus(404)
    #define STATUS_UNAUTHORIZED    reqstatus(401)

    struct Context{
        std::shared_ptr<entity::User> user;
        Json::Value req;
        int connectionID;
    };

    std::string Handle(int sd, std::string message);
    void Disconnect(int sd);
}