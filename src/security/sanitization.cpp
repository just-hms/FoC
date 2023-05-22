#include "security.h"

static char whitelistUNAME[] = "abcdefghijklmnopqrstuvwxyz""ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static char whitelistPWD[] = "abcdefghijklmnopqrstuvwxyz""ABCDEFGHIJKLMNOPQRSTUVWXYZ""0123456789""!\"#$%&\'()*+,-./:;<=>?@[]\\^_{}|~";
static char whiteAMOUNT[] = "0123456789";
static char *whitelist[] = {whitelistUNAME, whitelistPWD, whiteAMOUNT};

bool sec::sanitize(std::string data, unsigned int index) {
    unsigned int size = data.size();
    if(size >= sec::MAX_SANITIZATION_LEN || size == 0) {
        return false;
    }
    
    if(data.find_first_not_of(whitelist[index]) != std::string::npos) return false;

    return true;
}