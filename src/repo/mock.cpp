#include "repo.h"
#include <memory>
#include <vector>

std::tuple<std::shared_ptr<entity::User>, entity::Error> repo::MockBankRepo::Login(std::string username, std::string password){
    if (username!="kek" || password != "kekkone!"){
        return {nullptr, entity::ERR_BROKEN};
    }

    auto us = std::make_shared<entity::User>();
    us->username = "kek";
    us->password = "kekkone!";
    return {us, entity::ERR_OK};
}


std::tuple<entity::Balance, entity::Error> repo::MockBankRepo::Balance(std::string USER_ID){
    return {{10, "kek"}, entity::ERR_OK};
}

std::tuple<bool, entity::Error> repo::MockBankRepo::Transfer(entity::Transaction * t){
    return {true, entity::ERR_OK};
}

std::tuple<entity::History, entity::Error> repo::MockBankRepo::History(std::string username){
    auto transactions = entity::History();

    transactions.push_back(entity::Transaction{
        .from = "kek",
        .to = "giovanni",
        .amount = 10.5,
    });

    transactions.push_back(entity::Transaction{
        .from = "miao",
        .to = "kek",
        .amount = 10.5,
    });

    return {transactions, entity::ERR_OK};
}
