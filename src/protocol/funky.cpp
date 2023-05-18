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
    std::shared_ptr<protocol::FunkySecuritySuite> suite;

    //  1. Receive handshake message from client, build new AsymCrypt obj 
    auto [res, err1] = protocol::RawReceive(sd);
    if (err1 != entity::ERR_OK){
        return {nullptr, err1};
    }

    suite->asy = sec::AsymCrypt(
        "~/repos/FoC/server_privk.pem", 
        "", 
        "secret"
    );

    auto message = suite->asy.decrypt(res);
    char username[entity::USERNAME_MAX_LEN];
    size_t timestamp;
    sscanf((char*) message.data(), "%19s|%zu\n", username, &timestamp);   //test overflow

    auto ts = time(NULL);
    if(timestamp > ts - entity::ACCEPTANCE_WINDOW) return {nullptr, err1};   //message has to be sent in the last ACCEPTANCE_WINDOW seconds

    suite->asy.setPeerKey(std::string(username)+PUBK);  //missing initial path, should be provided when building FunkyProtocol obj
    
    //  2. Generate and send to client DH parameters
    EVP_PKEY *paramsDH = nullptr, *rightDH = nullptr;
    auto DH = sec::genDHparam(paramsDH);
    auto out = suite->asy.encrypt(DH);

    auto err = protocol::RawSend(sd, out);
    if (err != entity::ERR_OK){
        return {nullptr, err};
    }

    //  3. Generate and send to client the server's DH public key
    sec::genDH(rightDH, paramsDH);
    auto encodedRightDH = sec::encodePublicKey(rightDH);
    out = suite->asy.encrypt(encodedRightDH);

    err = protocol::RawSend(sd, out);
    if (err != entity::ERR_OK){
        return {nullptr, err};
    }

    // 4. Receive client's DH public key
    auto [res4, err4] = protocol::RawReceive(sd);
    if (err4 != entity::ERR_OK){
        return {nullptr, err4};
    }

    auto encodedLeftDH = suite->asy.decrypt(res4);
    auto leftDH = sec::decodePublicKey(encodedLeftDH);

    // 5. Derivate secret, generate key for SymCrypt
    auto secret = sec::derivateDH(rightDH, leftDH);
    auto key = sec::keyFromSecret(secret);

    suite->sym = sec::SymCrypt(key);

    //  6. Creation and delivery of MAC symmetric key
    suite->mac = sec::Hmac();
    unsigned char *MACk = suite->mac.getKey();
    out = suite->sym.encrypt(std::vector<uint8_t>(MACk, MACk + 16));
    err = protocol::RawSend(sd, out);
    if (err != entity::ERR_OK){
        return {nullptr, err};
    }

    //  7. MAC of the all session
    std::vector<uint8_t> HSsession;
    HSsession.insert(HSsession.end(), message.begin(), message.end());
    HSsession.insert(HSsession.end(), DH.begin(), DH.end());
    HSsession.insert(HSsession.end(), encodedRightDH.begin(), encodedRightDH.end());
    HSsession.insert(HSsession.end(), encodedLeftDH.begin(), encodedLeftDH.end());
    HSsession.insert(HSsession.end(), secret.begin(), secret.end());

    auto hashedSession = suite->mac.MAC(HSsession);
    auto [res7, err7] = protocol::RawReceive(sd);
    if (err7 != entity::ERR_OK){
        return {nullptr, err7};
    }

    if(suite->sym.decrypt(res) != hashedSession) return {nullptr, err1};


    return {suite, entity::ERR_OK};
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

    auto err = protocol::RawSend(sd, out);
    if (err != entity::ERR_OK){
        return {nullptr, err};
    }

    // 2. Receive DH parameter
    auto [res, err1] = protocol::RawReceive(sd);
    if (err1 != entity::ERR_OK){
        return {nullptr, err1};
    }

    auto DH = suite->asy.decrypt(res);
    auto paramsDH = sec::retrieveDHparam(DH);

    // 3. Receive rightDH key
    auto [re2, err2] = protocol::RawReceive(sd);
    if (err2 != entity::ERR_OK){
        return {nullptr, err1};
    }

    auto encodedRightDH = suite->asy.decrypt(res);
    auto rightDH = sec::decodePublicKey(encodedRightDH);

    // 4. Send leftDH key
    EVP_PKEY *leftDH = nullptr;
    
    sec::genDH(leftDH, paramsDH);

    auto encodedLeftDH = sec::encodePublicKey(leftDH);
    out = suite->asy.encrypt(encodedLeftDH);

    err = protocol::RawSend(sd, out);
    if (err != entity::ERR_OK){
        return {nullptr, err};
    }

    // 5. Derivate secret, generate key for SymCrypt
    auto secret = sec::derivateDH(leftDH, rightDH);
    auto key = sec::keyFromSecret(secret);

    suite->sym = sec::SymCrypt(key);


    // 6. Receive the key to be used in MAC 
    auto [res6, err6] = protocol::RawReceive(sd);
    if (err6 != entity::ERR_OK){
        return {nullptr, err1};
    }

    auto decryptedMacKey = suite->sym.decrypt(res6);
    suite->mac = sec::Hmac(decryptedMacKey);
    
    //  7. MAC of all the session
    std::vector<uint8_t> HSsession;
    HSsession.insert(HSsession.end(), message.begin(), message.end());
    HSsession.insert(HSsession.end(), DH.begin(), DH.end());
    HSsession.insert(HSsession.end(), encodedRightDH.begin(), encodedRightDH.end());
    HSsession.insert(HSsession.end(), encodedLeftDH.begin(), encodedLeftDH.end());
    HSsession.insert(HSsession.end(), secret.begin(), secret.end());

    auto hashedSession = suite->mac.MAC(HSsession);
    out = suite->sym.encrypt(hashedSession);
    err = protocol::RawSend(sd, out);
    if (err != entity::ERR_OK){
        return {nullptr, err};
    }

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
