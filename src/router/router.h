#ifndef ROUTER_H
#define ROUTER_H

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "./../entity/entity.h"
#include "./../network/network.h"

namespace router {

typedef int reqstatus;
constexpr reqstatus STATUS_OK = 200;
constexpr reqstatus STATUS_BAD_REQUEST = 400;
constexpr reqstatus STATUS_NOT_FOUND = 404;
constexpr reqstatus STATUS_UNAUTHORIZED = 401;
constexpr reqstatus STATUS_INTERNAL_ERROR = 500;

struct Context {
    std::shared_ptr<entity::User> user;
    Json::Value req;
    int connectionID;
};

class MockPongRouter : public net::IRouter {
   public:
    virtual std::string Handle(int sd, std::string message);
    void Disconnect(int sd);
};

class IRepo {
   public:
    virtual std::tuple<std::shared_ptr<entity::User>, entity::Error> Login(
        std::string username, std::string password) = 0;
    virtual std::tuple<entity::Balance, entity::Error> Balance(
        std::string USER_ID) = 0;
    virtual std::tuple<bool, entity::Error> Transfer(
        entity::Transaction *t) = 0;
    virtual std::tuple<entity::History, entity::Error> History(
        std::string USER_ID) = 0;
};

class Router : public net::IRouter {
   private:
    // map of currently logged users SD -> entity:User
    std::unordered_map<int, std::shared_ptr<entity::User>> users;

    // repository
    IRepo *repo;

    // handlers
    Json::Value Login(Context *ctx);
    Json::Value Balance(Context *ctx);
    Json::Value Transfer(Context *ctx);
    Json::Value History(Context *ctx);

   public:
    Router(IRepo *repo);
    virtual std::string Handle(int sd, std::string message);
    virtual void Disconnect(int sd);
};
}  // namespace router

#endif