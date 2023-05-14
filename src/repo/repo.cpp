#include "repo.h"
#include <vector>

entity::User* repo::Login(std::string username, std::string password){
    if (username!="kek" && password != "kek"){
        return nullptr;
    }

    auto us = new entity::User{};
    us->ID = "wejhb872bu9324";
    us->username = "kek";
    us->password = "kek";
    return us;
}

entity::User* repo::GetByID(std::string userID){
    if (userID == "wejhb872bu9324"){
        return nullptr;
    }

    auto us = new entity::User{};
    us->ID = "wejhb872bu9324";
    us->username = "kek";
    us->password = "kek";
    return us;
}

int repo::Balance(std::string USER_ID){
    return 10;
}

bool repo::Transfer(std::string to, float amount){
    return true;
}

entity::History repo::History(std::string username){
    auto transactions = entity::History();

    transactions.push_back(entity::Transaction{
        .from="kek",
        .to="giovanni",
        .amount = 10.5,
    });

    return transactions;
}
