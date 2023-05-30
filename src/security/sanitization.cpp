#include "security.h"
#include <regex>

static char whitelistUNAME[] = "abcdefghijklmnopqrstuvwxyz""ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static char whitelistPWD[] = "abcdefghijklmnopqrstuvwxyz""ABCDEFGHIJKLMNOPQRSTUVWXYZ""0123456789""!\"#$%&\'()*+,-./:;<=>?@[]\\^_{}|~";
static char whiteAMOUNT[] = "0123456789";
static char *whitelist[] = {whitelistUNAME, whitelistPWD, whiteAMOUNT};

bool isFloatWithTwoDecimalPlaces(const std::string& str) {
    // Regular expression pattern
    std::regex pattern("^[-+]?[0-9]+([.][0-9]{1,2})?$");

    // Check if the string matches the pattern
    return std::regex_match(str, pattern);
}

bool sec::sanitize(std::string data, unsigned int index) {
    unsigned int size = data.size();
    if(size >= sec::MAX_SANITIZATION_LEN || size == 0) {
        return false;
    }

    if(index == 1 && size < 8) return false;
    if(index == 2 && data == "0") return false;
    if(index == 2 && !(isFloatWithTwoDecimalPlaces(data))) return false;

    if(data.find_first_not_of(whitelist[index]) != std::string::npos) return false;

    return true;
}