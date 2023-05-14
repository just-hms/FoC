#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <vector>

namespace entity {
    
    typedef int Error;

    #define ERR_OK                  Error(+0)
    #define ERR_NOT_FOUND           Error(-1)
    #define ERR_TIMEOUT             Error(-2)
    #define ERR_BROKEN              Error(-3)
    #define ERR_MESSAGE_TOO_LONG    Error(-5)

    struct Transaction{
        std::string from;
        std::string to;
        float amount;
    };

    struct User{
        std::string ID;
        std::string username;
        std::string password;
    };

    typedef std::vector<Transaction> History;

    Error StatusCodeFromCSocketErrorCodes(int code);

}

#endif
