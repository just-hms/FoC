#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>

#include <fstream>
#include <memory>
#include <type_traits>
#include <vector>

#include "repo.h"

namespace repo {

BankRepo::BankRepo(std::string folder_path, std::string secret,
                   int historyLen) {
    this->folder_path = folder_path;
    this->sym = std::make_shared<sec::SymCrypt>(
        sec::SymCrypt(std::vector<uint8_t>(secret.begin(), secret.end())));
    this->historyLen = historyLen;
}

entity::Error BankRepo::Create(entity::User *u) {
    auto filename = this->folder_path + "users.json";

    std::ifstream ifs(filename);
    Json::Value usersJson;

    // if the file exists
    if (ifs.is_open()) {
        // read out the users into obj
        std::stringstream buf;
        buf << ifs.rdbuf();
        ifs.close();
        auto encryptedUsers = buf.str();

        if (encryptedUsers != "") {
            auto [usersValue, err] = this->sym->decrypt(std::vector<uint8_t>(
                encryptedUsers.begin(), encryptedUsers.end()));
            if (err != entity::ERR_OK) {
                return err;
            }

            Json::Reader reader;
            reader.parse(std::string(usersValue.begin(), usersValue.end()),
                         usersJson);

            // if the user already exists return error
            auto userJson = usersJson[u->username];
            if (!userJson.empty()) {
                return entity::ERR_ALREADY_EXISTS;
            }
        }
    }

    // sanityze
    if (!sanitize::isPassword(u->password) ||
        !sanitize::isUsername(u->username)) {
        return entity::ERR_UNATORIZED;
    }

    Json::Value createUserJson;
    createUserJson["accountID"] = u->balance.accountID;
    createUserJson["balance"] = u->balance.amount;
    createUserJson["username"] = u->username;
    auto [hashedPassword, err] = sec::HashAndSalt(u->password);
    if (err != entity::ERR_OK) {
        return err;
    }
    createUserJson["password"] = hashedPassword;

    // add user to the users List
    usersJson[u->username] = createUserJson;

    Json::StreamWriterBuilder builder;
    std::string usersString = Json::writeString(builder, usersJson);

    auto [encryptedValue, errEnc] = this->sym->encrypt(
        std::vector<uint8_t>(usersString.begin(), usersString.end()));
    if (errEnc != entity::ERR_OK) {
        return errEnc;
    }

    // write down the encrypted list of users
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        return entity::ERR_BROKEN;
    }
    ofs << std::string(encryptedValue.begin(), encryptedValue.end());
    ofs.close();

    return entity::ERR_OK;
}

std::tuple<entity::User, entity::Error> BankRepo::getUserByName(
    std::string username) {
    auto filename = this->folder_path + "users.json";

    std::ifstream ifs(filename);
    std::stringstream buf;
    buf << ifs.rdbuf();
    ifs.close();
    auto encryptedUsers = buf.str();

    // decrypt file
    auto [usersValue, err] = this->sym->decrypt(
        std::vector<uint8_t>(encryptedUsers.begin(), encryptedUsers.end()));

    // put the string into a json
    Json::Reader reader;
    Json::Value obj;

    reader.parse(std::string(usersValue.begin(), usersValue.end()), obj);

    // get the user
    auto userJson = obj[username];
    if (userJson.empty()) {
        return {entity::User{}, entity::ERR_NOT_FOUND};
    }

    auto us = entity::User{.username = userJson["username"].asString(),
                           .password = userJson["password"].asString(),
                           .balance = entity::Balance{
                               .amount = userJson["balance"].asFloat(),
                               .accountID = userJson["accountID"].asString()}};

    return {us, entity::ERR_OK};
}

std::tuple<std::shared_ptr<entity::User>, entity::Error> BankRepo::Login(
    std::string username, std::string password) {
    auto [us, err] = this->getUserByName(username);
    if (err != entity::ERR_OK) {
        return {nullptr, entity::ERR_UNATORIZED};
    }

    // check the password
    auto hashedPassword = us.password;
    if (!sec::VerifyHash(hashedPassword, password)) {
        return {nullptr, entity::ERR_UNATORIZED};
    }

    // return the user values

    auto user = std::make_shared<entity::User>();
    user->username = us.username;
    user->password = us.password;
    user->balance = us.balance;

    return {user, entity::ERR_OK};
}

std::tuple<entity::Balance, entity::Error> BankRepo::Balance(
    std::string username) {
    auto [us, err] = this->getUserByName(username);
    if (err != entity::ERR_OK) {
        return {entity::Balance{}, entity::ERR_NOT_FOUND};
    }

    // return the user values
    return {us.balance, entity::ERR_OK};
}

std::tuple<bool, entity::Error> BankRepo::Transfer(entity::Transaction *t) {
    if (t->from == t->to) {
        return {false, entity::ERR_NOT_FOUND};
    }
    auto [us1, err1] = this->getUserByName(t->to);
    if (err1 != entity::ERR_OK) {
        return {false, entity::ERR_NOT_FOUND};
    }

    if (t->amount < 0) {
        return {false, entity::ERR_BROKEN};
    }

    // check if the current balance is above the requested one
    auto [senderBalance, err] = this->Balance(t->from);
    if (err != entity::ERR_OK || senderBalance.amount < t->amount) {
        return {false, entity::ERR_BROKEN};
    }

    auto [senderHistory, errH] = this->History(t->from);
    if (errH != entity::ERR_OK) {
        return {false, entity::ERR_BROKEN};
    }

    auto [receiverHistory, errHistory2] = this->History(t->to);
    if (errH != entity::ERR_OK) {
        return {false, entity::ERR_BROKEN};
    }

    // add new transfer to the history
    senderHistory.push_back(*t);
    err = this->updateHistory(t->from, &senderHistory);
    if (err != entity::ERR_OK) {
        return {false, entity::ERR_BROKEN};
    }

    receiverHistory.push_back(*t);
    err = this->updateHistory(t->to, &receiverHistory);
    if (err != entity::ERR_OK) {
        return {false, entity::ERR_BROKEN};
    }

    // update the bank accounts of the interest users
    err = this->updateBalances(t);
    if (err != entity::ERR_OK) {
        return {false, entity::ERR_BROKEN};
    }

    return {true, entity::ERR_OK};
}

entity::Error BankRepo::updateBalances(entity::Transaction *t) {
    auto filename = this->folder_path + "users.json";

    std::ifstream ifs(filename);
    std::stringstream buf;
    buf << ifs.rdbuf();
    ifs.close();
    auto encryptedUsers = buf.str();

    // decrypt file
    auto [usersValue, err] = this->sym->decrypt(
        std::vector<uint8_t>(encryptedUsers.begin(), encryptedUsers.end()));

    // put the decrypted value into json
    Json::Reader reader;
    Json::Value obj;
    reader.parse(std::string(usersValue.begin(), usersValue.end()), obj);

    // update the balances
    obj[t->from]["balance"] = obj[t->from]["balance"].asFloat() - t->amount;
    obj[t->to]["balance"] = obj[t->to]["balance"].asFloat() + t->amount;

    // put the json into string
    Json::StreamWriterBuilder builder;
    std::string usersString = Json::writeString(builder, obj);

    // encrypt the string
    auto [encryptedValue, errEnc] = this->sym->encrypt(
        std::vector<uint8_t>(usersString.begin(), usersString.end()));
    if (errEnc != entity::ERR_OK) {
        return errEnc;
    }

    // write down the updated list of users
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        return entity::ERR_BROKEN;
    }
    ofs << std::string(encryptedValue.begin(), encryptedValue.end());
    ofs.close();

    return entity::ERR_OK;
}

entity::Error BankRepo::updateHistory(std::string username,
                                      entity::History *h) {
    if (h->size() > this->historyLen) {
        h->erase(h->begin());
    }

    Json::Value out;

    for (const auto &transfer : *h) {
        Json::Value jsonTransfer;

        jsonTransfer["amount"] = transfer.amount;
        jsonTransfer["from"] = transfer.from;
        jsonTransfer["to"] = transfer.to;

        out.append(jsonTransfer);
    }

    Json::StreamWriterBuilder builder;
    std::string str = Json::writeString(builder, out);

    auto [encrypted, err] =
        this->sym->encrypt(std::vector<uint8_t>(str.begin(), str.end()));
    if (err != entity::ERR_OK) {
        return err;
    }

    // write down the encrypted history
    std::ofstream ofs(this->folder_path + "transfers/" + username + ".json");
    if (!ofs.is_open()) {
        return entity::ERR_BROKEN;
    }
    ofs << std::string(encrypted.begin(), encrypted.end());
    ofs.close();

    return entity::ERR_OK;
}

std::tuple<entity::History, entity::Error> BankRepo::History(
    std::string username) {
    std::ifstream ifs(this->folder_path + "transfers/" + username + ".json");

    // if the file does not exists the history is empty
    if (!ifs.is_open()) {
        return {entity::History(), entity::ERR_OK};
    }

    std::stringstream buf;
    buf << ifs.rdbuf();
    auto encryptedTransfers = buf.str();

    auto [transfersValue, err] = this->sym->decrypt(std::vector<uint8_t>(
        encryptedTransfers.begin(), encryptedTransfers.end()));
    if (err != entity::ERR_OK) {
        return {entity::History(), entity::ERR_BROKEN};
    }

    Json::Reader reader;
    Json::Value obj;

    reader.parse(std::string(transfersValue.begin(), transfersValue.end()),
                 obj);

    auto res = entity::History();

    for (auto el : obj) {
        entity::Transaction t;
        t.amount = el["amount"].asFloat();
        t.from = el["from"].asString();
        t.to = el["to"].asString();
        res.push_back(t);
    }

    return {res, entity::ERR_OK};
}
}  // namespace repo
