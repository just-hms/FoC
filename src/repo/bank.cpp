#include "repo.h"


repo::BankRepo::BankRepo(std::string path){
    this->path = path;
}

std::tuple<std::shared_ptr<entity::User>, entity::Error> repo::BankRepo::Login(std::string username, std::string password){
    std::ifstream ifs(path + "/users.json");
    Json::Reader reader;
    Json::Value obj;

    reader.parse(ifs, obj);

    // get the user
    auto userJson = obj[username];
    if (userJson.empty()){
        return {nullptr, entity::ERR_NOT_FOUND};
    }

    // check the password
    auto hashedPassword = userJson["password"].asString();
    if (!sec::VerifyHash(hashedPassword,password)){
        return {nullptr, entity::ERR_NOT_FOUND};        
    }

    // return the user values

    auto us = std::make_shared<entity::User>();
    us->ID = userJson["ID"].asString();
    us->username = userJson["username"].asString();

    return {us, entity::ERR_OK};
}


std::tuple<int, entity::Error> repo::MockBankRepo::Balance(std::string USER_ID){
}

std::tuple<bool, entity::Error> repo::MockBankRepo::Transfer(std::string USER_ID, std::string to, float amount){
}

std::tuple<entity::History, entity::Error> repo::MockBankRepo::History(std::string username){
}
