#include "./../security/security.h"
#include "./../config/config.h"

int main(){
    config::Config cfg;

    sec::generateRSAkeys("./data/server", cfg.Secret, 4096);
    sec::generateRSAkeys("./data/client", cfg.Secret, 4096);
}