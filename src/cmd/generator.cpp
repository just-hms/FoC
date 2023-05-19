#include "./../security/security.h"

int main(){
    sec::generateRSAkeys("../../data/server", "secret", 4096);
    sec::generateRSAkeys("../../data/client", "secret", 4096);
}