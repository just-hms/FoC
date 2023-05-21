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
        virtual std::tuple<int, entity::Error> Balance(std::string USER_ID);
        virtual std::tuple<bool, entity::Error> Transfer(std::string USER_ID, std::string to, float amount);
        virtual std::tuple<entity::History,entity::Error> History(std::string USER_ID);
    };

    class BankRepo : public router::IRepo{
    private:
        std::string path;
    public:
        BankRepo(std::string url);
        virtual std::tuple<std::shared_ptr<entity::User>, entity::Error> Login(std::string username, std::string password);
        virtual std::tuple<int, entity::Error> Balance(std::string USER_ID);
        virtual std::tuple<bool, entity::Error> Transfer(std::string USER_ID, std::string to, float amount);
        virtual std::tuple<entity::History,entity::Error> History(std::string USER_ID);
    };
}

#endif