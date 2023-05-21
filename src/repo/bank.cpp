#include "repo.h"
#include <fstream>
#include <jsoncpp/json/writer.h>
#include <memory>
#include <type_traits>
#include <vector>

repo::BankRepo::BankRepo(std::string path, std::string secret){
    this->path = path;
    this->sym = std::make_shared<sec::SymCrypt>(sec::SymCrypt(secret));
}

std::tuple<std::shared_ptr<entity::User>, entity::Error> repo::BankRepo::Login(std::string username, std::string password){
    std::ifstream ifs(this->path + "/users.json");
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


std::tuple<entity::Balance, entity::Error> repo::BankRepo::Balance(std::string username){
    std::ifstream ifs(path + "/users.json");
    Json::Reader reader;
    Json::Value obj;

    reader.parse(ifs, obj);

    // get the user
    auto userJson = obj[username];
    if (userJson.empty()){
        return {{0, ""}, entity::ERR_NOT_FOUND};
    }

    // TODO return also the account id
    // return the user values
    return {
        {
            userJson["balance"].asFloat(),
            userJson["accountID"].asString(),
        }, 
        entity::ERR_OK
    };
}

std::tuple<bool, entity::Error> repo::BankRepo::Transfer(std::string username, std::string to, float amount){
    if (amount < 0){
        return {false, entity::ERR_BROKEN}; 
    }

    auto [curbalance, err] = this->Balance(username);
    if (curbalance.amount < amount){
        return {false, entity::ERR_BROKEN};         
    }
    auto [receiverBalance, err2] = this->Balance(username);

    // TODO add tranfer logic
    //  - update each amount
    //  - update transfer history
    
    return {true, entity::ERR_OK};
}

std::tuple<entity::History, entity::Error> repo::BankRepo::History(std::string username){
    std::ifstream ifs(path + "/transfers.json");
    Json::Reader reader;
    Json::Value obj;

    reader.parse(ifs, obj);

    auto encryptedTransfers= obj[username].asString();
    if (encryptedTransfers.empty()){
        return {{}, entity::ERR_NOT_FOUND};
    }

    auto [tranfersJson, err] = this->sym->decrypt(
        std::vector<uint8_t>(
            encryptedTransfers.begin(), 
            encryptedTransfers.end()
        )
    );

    if (err != entity::ERR_OK){
        return {{}, entity::ERR_BROKEN};
    }

    auto tranfersJsonString = std::string(tranfersJson.begin(), tranfersJson.end());

    reader.parse(tranfersJsonString, obj);
    auto res = entity::History();

    for (auto el : obj){
        entity::Transaction t;
        t.amount = el["amount"].asFloat();
        t.from = el["from"].asString();
        t.to = el["to"].asString();
        res.push_back(t);
    } 

    return {res, entity::ERR_OK};
}
