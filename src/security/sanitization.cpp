#include "security.h"
#include <regex>

namespace sanitize {


    bool isCurrency(const std::string& str) {
        if(str.size() >= sec::MAX_SANITIZATION_LEN || str.size() == 0) {
            return false;
        }
        // Matches:
        //  - 10
        //  - 10.10
        //  - 10.5
        std::regex pattern("^[-+]?[0-9]+([.][0-9]{1,2})?$");

        // Check if the string matches the pattern
        return std::regex_match(str, pattern);
    }

    bool isUsername(const std::string& str) {
        if(str.size() >= sec::MAX_SANITIZATION_LEN || str.size() == 0) {
            return false;
        }
        // Any alphanumerical charcter and _ and starts with a letter
        std::regex pattern("^[a-zA-Z][a-zA-Z0-9_]*$");

        // contains any 
        return std::regex_match(str, pattern);
    }


    bool isPassword(const std::string& str) {
        if(str.size() >= sec::MAX_SANITIZATION_LEN || str.size() == 0 || str.size() < 8) {
            return false;
        }
        
        // allow alphanumerical characters and some special ones
        std::regex pattern("^[a-zA-Z0-9!\"#$%&'()*+,-./:;<=>?@[\\]^_{}|~]+$");

        // Check if the string matches the pattern
        return std::regex_match(str, pattern);
    }
}