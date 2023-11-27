#ifndef REPO_H
#define REPO_H

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "./../entity/entity.h"
#include "./../router/router.h"
#include "./../security/security.h"
#include "./../uuid/uuid.h"

namespace repo {
class MockBankRepo : public router::IRepo {
   public:
    virtual std::tuple<std::shared_ptr<entity::User>, entity::Error> Login(
        std::string username, std::string password);
    virtual std::tuple<entity::Balance, entity::Error> Balance(
        std::string username);
    virtual std::tuple<bool, entity::Error> Transfer(entity::Transaction* t);
    virtual std::tuple<entity::History, entity::Error> History(
        std::string username);
};

class BankRepo : public router::IRepo {
   private:
    std::string folder_path;
    std::shared_ptr<sec::SymCrypt> sym;
    int historyLen;

   private:
    std::tuple<entity::User, entity::Error> getUserByName(std::string username);
    entity::Error updateHistory(std::string username, entity::History* h);
    entity::Error updateBalances(entity::Transaction* t);

   public:
    BankRepo(std::string folder_path, std::string secret, int historyLen);
    virtual std::tuple<std::shared_ptr<entity::User>, entity::Error> Login(
        std::string username, std::string password);
    virtual std::tuple<entity::Balance, entity::Error> Balance(
        std::string username);
    virtual std::tuple<bool, entity::Error> Transfer(entity::Transaction* t);
    virtual std::tuple<entity::History, entity::Error> History(
        std::string username);

    entity::Error Create(entity::User* u);
};
}  // namespace repo

#endif