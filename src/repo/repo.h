#include <string>
#include <vector>
#include "./../entity/entity.h"
#include <memory>

namespace repo {
    std::shared_ptr<entity::User> Login(std::string username, std::string password);
    int Balance(std::string USER_ID);
    bool Transfer(std::string USER_ID, std::string to, float amount);
    entity::History History(std::string USER_ID);
}