#include "security.h"

static char whitelistUNAME[] = "abcdefghijklmnopqrstuvwxyz""ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static char whitelistPWD[] = "abcdefghijklmnopqrstuvwxyz""ABCDEFGHIJKLMNOPQRSTUVWXYZ""0123456789""!\"#$%&\'()*+,-./:;<=>?@[]\\^_{}|~";
static char whiteAMOUNT[] = "0123456789";
static char *whitelist[] = {whitelistUNAME, whitelistPWD, whiteAMOUNT};

bool sec::sanitize(std::string data, unsigned int index) {
    if(data.size() >= sec::MAX_SANITIZATION_LEN) {
        return false;
    }
    
    if(data.find_first_not_of(whitelist[index]) != std::string::npos) return false;

    return true;
}