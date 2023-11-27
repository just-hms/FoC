#ifndef ENTITY_H
#define ENTITY_H

#include <iostream>
#include <string>
#include <vector>

namespace entity {

typedef int Error;

constexpr Error ERR_OK = +0;
constexpr Error ERR_NOT_FOUND = -1;
constexpr Error ERR_TIMEOUT = -2;
constexpr Error ERR_BROKEN = -3;
constexpr Error ERR_MESSAGE_TOO_LONG = -5;
constexpr Error ERR_DURING_HANDSHAKE = -6;
constexpr Error ERR_FILE_NOT_FOUND = -7;
constexpr Error ERR_ALREADY_EXISTS = -8;
constexpr Error ERR_UNATORIZED = -9;

constexpr size_t ACCEPTANCE_WINDOW = 5;

struct Transaction {
    std::string from;
    std::string to;
    float amount;
};

struct Balance {
    float amount;
    std::string accountID;
};

struct User {
    std::string username;
    std::string password;
    Balance balance;
};

typedef std::vector<Transaction> History;

Error StatusCodeFromCSocketErrorCodes(int code);
void Log(std::string message);

}  // namespace entity

#endif
