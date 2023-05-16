#ifndef REPO_H
#define REPO_H

#include <string>
#include <vector>
#include "./../entity/entity.h"
#include "./../router/router.h"
#include <memory>

namespace repo {
    class MockRepo : public router::IRepo{
        virtual std::shared_ptr<entity::User> Login(std::string username, std::string password);
        virtual int Balance(std::string USER_ID);
        virtual bool Transfer(std::string USER_ID, std::string to, float amount);
        virtual entity::History History(std::string USER_ID);
    };
}

#endif