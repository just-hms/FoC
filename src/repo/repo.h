#include <string>
#include <vector>
#include "./../entity/entity.h"


namespace repo {
    entity::User* Login(std::string username, std::string password);
    entity::User* GetByID(std::string userID);
    int Balance(std::string USER_ID);
    bool Transfer(std::string USER_ID, std::string to, float amount);
    entity::History History(std::string USER_ID);
}