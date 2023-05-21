#ifndef REPO_H
#define REPO_H

#include <string>
#include <vector>
#include "./../entity/entity.h"
#include "./../router/router.h"
#include "./../security/security.h"
#include <memory>
#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>


namespace repo {
    class MockBankRepo : public router::IRepo{
    public:
        virtual std::tuple<std::shared_ptr<entity::User>, entity::Error> Login(std::string username, std::string password);
        virtual std::tuple<entity::Balance, entity::Error> Balance(std::string username);
        virtual std::tuple<bool, entity::Error> Transfer(std::string username, std::string to, float amount);
        virtual std::tuple<entity::History,entity::Error> History(std::string username);
    };

    class BankRepo : public router::IRepo{
    private:
        std::string path;
        std::shared_ptr<sec::SymCrypt> sym;
    public:
        BankRepo(std::string url, std::string secret);
        virtual std::tuple<std::shared_ptr<entity::User>, entity::Error> Login(std::string username, std::string password);
        virtual std::tuple<entity::Balance, entity::Error> Balance(std::string username);
        virtual std::tuple<bool, entity::Error> Transfer(std::string username, std::string to, float amount);
        virtual std::tuple<entity::History,entity::Error> History(std::string username);
    };
}

#endif