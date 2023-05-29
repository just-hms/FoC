#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <vector>
#include <iostream>


namespace entity {
    
    typedef int Error;

    constexpr Error ERR_OK =                +0;
    constexpr Error ERR_NOT_FOUND =         -1;
    constexpr Error ERR_TIMEOUT =           -2;
    constexpr Error ERR_BROKEN =            -3;
    constexpr Error ERR_MESSAGE_TOO_LONG =  -5;
    constexpr Error ERR_DURING_HANDSHAKE =  -5;
    constexpr Error ERR_FILE_NOT_FOUND =    -6;
    constexpr Error ERR_ALREADY_EXISTS =    -7;
    
    constexpr size_t USERNAME_MAX_LEN =     20;
    constexpr size_t ACCEPTANCE_WINDOW =  5000;

    struct Transaction{
        std::string from;
        std::string to;
        float amount;
    };

    struct User{
        std::string username;
        std::string password;
    };

    struct Balance{
        float amount;
        std::string AccountID;
    };

    typedef std::vector<Transaction> History;

    Error StatusCodeFromCSocketErrorCodes(int code);
    void Log(std::string message);

}

#endif
