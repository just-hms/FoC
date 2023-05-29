#include "./../security/security.h"
#include "./../config/config.h"
#include "./../repo/repo.h"
#include "./../uuid/uuid.h"
#include <vector>

#define basePath std::string("./data/")

int main(){
    config::Config cfg;

    std::system(("rm " + basePath + "users.json").c_str());
    std::system(("mkdir -p " + basePath + "transfers").c_str());
    std::system(("mkdir -p " + basePath + "keys").c_str());

    auto users = std::vector<entity::User>{
        entity::User{
            .username = "patata",
            .password = "password",
            .balance = entity::Balance{
                .amount = 200,
                .accountID = uuid::New()
            },
        },
        entity::User{
            .username = "fragolino",
            .password = "password",
            .balance = entity::Balance{
                .amount = 100.50,
                .accountID = uuid::New()
            },
        },
        entity::User{
            .username = "poor",
            .password = "password",
            .balance = entity::Balance{
                .amount = 0,
                .accountID = uuid::New()
            },
        }
    }; 

    repo::BankRepo repo("./data/", cfg.Secret, cfg.HistoryLen);

    for (auto& us : users) {
        auto err = repo.Create(&us);

        if (err != entity::ERR_OK){
            std::cout << "error during the creation of "  << us.username << std::endl; 
            return 1;
        }
        err = sec::generateRSAkeys("./data/keys/" + us.username, cfg.Secret, 4096);
        
        if (err != entity::ERR_OK){
            std::cout << "error during the creation of the RSA key for "  << us.username << std::endl; 
            return 1;
        }
    }

    auto err = sec::generateRSAkeys("./data/keys/server", cfg.Secret, 4096);
    if (err != entity::ERR_OK){
        std::cout << "error during the creation of the RSA key for the server" << std::endl; 
        return 1;
    }
}