#include "./../security/security.h"

int main(){
    sec::generateRSAkeys("./data/server", "secret", 1024);
    sec::generateRSAkeys("./data/client", "secret", 1024);
}