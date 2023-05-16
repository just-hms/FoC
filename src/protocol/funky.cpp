#include "protocol.h"
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <span>
#include <string>
#include <utility>
#include <vector>


protocol::FunkyProtocol::FunkyProtocol(){}

std::tuple<std::shared_ptr<protocol::FunkySecuritySuite>,entity::Error> protocol::FunkyProtocol::RightHandshake(int sd){
    // TODO implement the RightHandshake
    //  - send the username to the server
    //  - dh using RSA generated keys
    return {};
}


std::tuple<std::shared_ptr<protocol::FunkySecuritySuite>,entity::Error> protocol::FunkyProtocol::LeftHandshake(int sd){
    std::shared_ptr<protocol::FunkySecuritySuite> suite;

    std::string name = "kek";

    // TODO add some way to get the file
    suite->asy = sec::AsymCrypt(
        "~/repos/FoC/client_privk.pem", 
        "~/repos/FoC/server_pubk.pem", 
        "secret"
    );

    // 1. Send a message with the username and the current time encryted with RSA

    auto now = time(NULL);
    auto message = name + "|" + std::to_string(now);

    auto out = suite->asy.encrypt(
        std::vector<uint8_t>(message.begin(), message.end())
    );

    auto err = protocol::RawSend(sd,out);
    if (err != entity::ERR_OK){
        return {{},err};
    }

    // 2. receive DH parameter

    auto [res, err1] = protocol::RawReceive(sd);
    if (err1 != entity::ERR_OK){
        return {{},err1};
    }

    auto encodedParamsDH = suite->asy.decrypt(res);
    auto paramsDH = sec::decodePublicKey(encodedParamsDH);

    // 3. receive rightDH key

    auto [re2, err2] = protocol::RawReceive(sd);
    if (err2 != entity::ERR_OK){
        return {{},err1};
    }

    auto encodedRightDH = suite->asy.decrypt(res);
    auto rightDH = sec::decodePublicKey(encodedParamsDH);

    // 4. send leftDH key

    EVP_PKEY *leftDH = nullptr;
    
    sec::genDH(leftDH, paramsDH);

    auto encodeLeftDH = sec::encodePublicKey(leftDH);
    out = suite->asy.encrypt(encodeLeftDH);

    err = protocol::RawSend(sd,out);
    if (err != entity::ERR_OK){
        return {{},err};
    }

    // 5. derivate the key

    auto secret = sec::derivateDH(leftDH, rightDH);
    auto key = sec::keyFromSecret(secret);

    suite->sym = sec::SymCrypt(key);


    // 6. receive the key to be used in mac 
    
    auto [res6, err6] = protocol::RawReceive(sd);
    if (err6 != entity::ERR_OK){
        return {{},err1};
    }

    auto decryptedMacKey = suite->sym.decrypt(res6);

    suite->mac = sec::Hmac(decryptedMacKey);
    
    // TODO
    //  - add hash of message history to check integrity

    return {suite, entity::ERR_OK};
}

entity::Error protocol::FunkyProtocol::Send(int sd, std::string message){
    
    std::shared_ptr<protocol::FunkySecuritySuite> secSuite = nullptr;
    
    auto it = this->sessions.find(sd);
    // if exists set it
    if (it == this->sessions.end()){
        auto [res, err] = this->LeftHandshake(sd);
        if (err != entity::ERR_OK){
            return err;
        }
        secSuite = res;
    } else{
        secSuite = it->second;
    }

    //  encrypt using session key
    auto out = secSuite->sym.encrypt(
        std::vector<uint8_t>(message.begin(), message.end())
    );

    // generating hash for integrity
    auto mac = secSuite->mac.MAC(out);
    
    // add hash to the message
    out.insert(out.end(), mac.begin(), mac.end());

    // call rawsend
    return protocol::RawSend(
        sd, 
        std::vector<uint8_t>(out.begin(),out.end())
    );
}

std::tuple<std::string,entity::Error> protocol::FunkyProtocol::Receive(int sd){

    std::shared_ptr<protocol::FunkySecuritySuite> secSuite = nullptr;
    
    auto it = this->sessions.find(sd);
    // if exists set it
    if (it == this->sessions.end()){
        auto [res, err] = this->RightHandshake(sd);
        if (err != entity::ERR_OK){
            return {"", err};
        }
        secSuite = res;
    } else{
        secSuite = it->second;
    }

    //  call raw receive
    auto [res, err] = protocol::RawReceive(sd);
    if (err != entity::ERR_OK){
        return {
            "",
            err
        };
    }

    //  extract hash and check for integrity

    //  decrypt using session key

    return {
        "",
        entity::ERR_BROKEN
    };
}
