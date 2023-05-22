#include "repo.h"
#include <memory>
#include <vector>

std::shared_ptr<entity::User> repo::MockRepo::Login(std::string username, std::string password){
    if (username != "kek" || password != "kek"){
        return nullptr;
    }

    auto us = std::make_shared<entity::User>();
    us->ID = "wejhb872bu9324";
    us->username = "kek";
    us->password = "kek";
    return us;
}


int repo::MockRepo::Balance(std::string USER_ID){
    return 10;
}

bool repo::MockRepo::Transfer(std::string USER_ID, std::string to, float amount){
    return true;
}

entity::History repo::MockRepo::History(std::string username){
    auto transactions = entity::History();

    transactions.push_back(entity::Transaction{
        .from="kek",
        .to="giovanni",
        .amount = 10.5,
    });

    transactions.push_back(entity::Transaction{
        .from="miao",
        .to="kek",
        .amount = 10.5,
    });

    return transactions;
}
