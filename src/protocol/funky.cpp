#include "protocol.h"
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

// TODO add a disconnect hook also for the protocols

protocol::FunkyProtocol::FunkyProtocol(protocol::FunkyOptions * opt){
    this->name = opt->name;
    this->peerName = opt->peerName;
    this->dataPath = opt->dataPath;
    this->secret = opt->secret;
}

std::tuple<protocol::FunkySecuritySuite,entity::Error> protocol::FunkyProtocol::RightHandshake(int sd){

    FunkySecuritySuite suite;

    //  1. Receive handshake message from client, build new AsymCrypt obj 

    auto [res, err1] = protocol::RawReceive(sd);
    if (err1 != entity::ERR_OK){
        return {FunkySecuritySuite{}, err1};
    }

    auto asy = sec::AsymCrypt(
        this->dataPath+std::string(this->name +"privk.pem"), 
        "", 
        this->secret
    );

    auto message = asy.decrypt(res);

    char username[entity::USERNAME_MAX_LEN];
    size_t timestamp;

    // TODO test overflow
    sscanf((char*) message.data(), "%19s %zu\n", username, &timestamp); 

    auto ts = time(NULL);
    //message has to be sent in the last ACCEPTANCE_WINDOW seconds

    if(!(timestamp > ts - entity::ACCEPTANCE_WINDOW)) {
        return {FunkySecuritySuite{}, entity::ERR_BROKEN};  
    }

    asy.setPeerKey(this->dataPath + std::string(username)+sec::PUBK);

    //  2. Generate and send to client a temporary symmetric key

    sec::sessionKey sk;
    RAND_bytes(sk.key, sec::SYMMLEN/8);
    RAND_bytes(sk.iv, 16);
    sec::SymCrypt symTMP(sk);
    std::vector<uint8_t> aesTMP(sec::SYMMLEN/8+16);
    memcpy(aesTMP.data(), &(sk.key)[0], sec::SYMMLEN/8);
    memcpy(aesTMP.data() + sec::SYMMLEN/8, &(sk.iv)[0], 16);

    auto out = asy.encrypt(aesTMP);

    auto err = protocol::RawSend(sd, out);
    if (err != entity::ERR_OK){
        return {FunkySecuritySuite{}, err};
    }

    //  3. Generate and send to client DH parameters
    EVP_PKEY *paramsDH = nullptr, *rightDH = nullptr;
    auto DH = sec::genDHparam(paramsDH);
    out = symTMP.encrypt(DH);

    err = protocol::RawSend(sd, out);
    if (err != entity::ERR_OK){
        return {FunkySecuritySuite{}, err};
    }

    //  4. Generate and send to client the server's DH public key
    sec::genDH(rightDH, paramsDH);
    auto encodedRightDH = sec::encodePublicKey(rightDH);
    out = symTMP.encrypt(encodedRightDH);

    err = protocol::RawSend(sd, out);
    if (err != entity::ERR_OK){
        return {FunkySecuritySuite{}, err};
    }

    // 5. Receive client's DH public key

    auto [res4, err4] = protocol::RawReceive(sd);
    if (err4 != entity::ERR_OK){
        return {FunkySecuritySuite{}, err4};
    }

    auto encodedLeftDH = symTMP.decrypt(res4);
    auto leftDH = sec::decodePublicKey(encodedLeftDH);

    // 6. Derivate secret, generate key for SymCrypt

    auto secret = sec::derivateDH(rightDH, leftDH);
    auto key = sec::keyFromSecret(secret);

    suite.sym = std::make_shared<sec::SymCrypt>(sec::SymCrypt(key));

    //  7. Creation and delivery of MAC symmetric key
    
    suite.mac = std::make_shared<sec::Hmac>(sec::Hmac());

    auto MACk = suite.mac->getKey();

    out = suite.sym->encrypt(MACk);
    err = protocol::RawSend(sd, out);
    if (err != entity::ERR_OK){
        return {FunkySecuritySuite{}, err};
    }

    //  8. MAC of the all session
    
    std::vector<uint8_t> HSsession;
    HSsession.insert(HSsession.end(), message.begin(), message.end());
    HSsession.insert(HSsession.end(), DH.begin(), DH.end());
    HSsession.insert(HSsession.end(), encodedRightDH.begin(), encodedRightDH.end());
    HSsession.insert(HSsession.end(), encodedLeftDH.begin(), encodedLeftDH.end());
    HSsession.insert(HSsession.end(), secret.begin(), secret.end());


    auto expectedHash = suite.mac->MAC(HSsession);
    
    auto [res7, err7] = protocol::RawReceive(sd);
    if (err7 != entity::ERR_OK){
        return {FunkySecuritySuite{}, err7};
    }

    auto hash = suite.sym->decrypt(res7);
    
    if(hash != expectedHash) {
        return {FunkySecuritySuite{}, entity::ERR_BROKEN};
    }

    return {suite, entity::ERR_OK};
}


std::tuple<protocol::FunkySecuritySuite,entity::Error> protocol::FunkyProtocol::LeftHandshake(int sd){

    FunkySecuritySuite suite;

    auto asy = sec::AsymCrypt(
        this->dataPath+std::string(this->name + "privk.pem"), 
        this->dataPath+std::string(this->peerName +"pubk.pem"), 
        this->secret
    );

    // 1. Send a message with the username and the current time encryted with RSA
    auto now = time(NULL);
    auto message = this->name + " " + std::to_string(now);

    auto out = asy.encrypt(
        std::vector<uint8_t>(message.begin(), message.end())
    );

    auto err = protocol::RawSend(sd, out);
    if (err != entity::ERR_OK){
        return {FunkySecuritySuite{}, err};
    }

    // 2. Receive temporary symmetric key
    auto [res, err1] = protocol::RawReceive(sd);
    if (err1 != entity::ERR_OK){
        return {FunkySecuritySuite{}, err1};
    }

    auto encodedSK = asy.decrypt(res);
    sec::sessionKey sk;
    memcpy(&(sk.key)[0], encodedSK.data(), sec::SYMMLEN/8);
    memcpy(&(sk.iv)[0], encodedSK.data()+sec::SYMMLEN/8, 16);

    sec::SymCrypt symTMP(sk);

    // 3. Receive DH parameters
    auto [res2, err2] = protocol::RawReceive(sd);
    if (err2 != entity::ERR_OK){
        return {FunkySecuritySuite{}, err2};
    }

    auto DH = symTMP.decrypt(res2);
    EVP_PKEY *paramsDH = sec::retrieveDHparam(DH);

    // 4. Receive rightDH key
    auto [res3, err3] = protocol::RawReceive(sd);
    if (err3 != entity::ERR_OK){
        return {FunkySecuritySuite{}, err3};
    }

    auto encodedRightDH = symTMP.decrypt(res3);

    // 5. Send leftDH key
    EVP_PKEY *leftDH = nullptr;
    
    sec::genDH(leftDH, paramsDH);

    auto encodedLeftDH = sec::encodePublicKey(leftDH);
    auto rightDH = sec::decodePublicKey(encodedRightDH);
    out = symTMP.encrypt(encodedLeftDH);

    err = protocol::RawSend(sd, out);
    if (err != entity::ERR_OK){
        return {FunkySecuritySuite{}, err};
    }

    // 6. Derivate secret, generate key for SymCrypt
    auto secret = sec::derivateDH(leftDH, rightDH);
    auto key = sec::keyFromSecret(secret);

    suite.sym = std::make_shared<sec::SymCrypt>(sec::SymCrypt(key));

    // 7. Receive the key to be used in MAC 
    auto [res6, err6] = protocol::RawReceive(sd);
    if (err6 != entity::ERR_OK){
        return {FunkySecuritySuite{}, err6};
    }
    
    auto decryptedMacKey = suite.sym->decrypt(res6);
    
    suite.mac = std::make_shared<sec::Hmac>(sec::Hmac(decryptedMacKey));

    //  8. MAC of all the session
    std::vector<uint8_t> HSsession;
    HSsession.insert(HSsession.end(), message.begin(), message.end());
    HSsession.insert(HSsession.end(), DH.begin(), DH.end());
    HSsession.insert(HSsession.end(), encodedRightDH.begin(), encodedRightDH.end());
    HSsession.insert(HSsession.end(), encodedLeftDH.begin(), encodedLeftDH.end());
    HSsession.insert(HSsession.end(), secret.begin(), secret.end());

    auto hash = suite.mac->MAC(HSsession);
    out = suite.sym->encrypt(hash);
    err = protocol::RawSend(sd, out);
    
    if (err != entity::ERR_OK){
        return {FunkySecuritySuite{}, err};
    }

    return {suite, entity::ERR_OK};
}

entity::Error protocol::FunkyProtocol::Send(int sd, std::string message){
    
    FunkySecuritySuite secSuite;
    
    auto it = this->sessions.find(sd);
    // if exists set it
    if(it == this->sessions.end()) {
        auto [res, err] = this->LeftHandshake(sd);
        sessions[sd] = res;
        
        if (err != entity::ERR_OK){
            return err;
        }
        secSuite = res;
    }
    else {
        secSuite = it->second;
    }

    //  encrypt using session key
    auto out = secSuite.sym->encrypt(
        std::vector<uint8_t>(message.begin(), message.end())
    );

    // generating hash for integrity
    auto mac = secSuite.mac->MAC(out);
    // add hash to the message
    out.insert(out.end(), mac.begin(), mac.end());
    // call rawsend
    return protocol::RawSend(sd, out);
}

std::tuple<std::string,entity::Error> protocol::FunkyProtocol::Receive(int sd){

    FunkySecuritySuite secSuite;
    
    auto it = this->sessions.find(sd);
    // if exists set it
    if(it == this->sessions.end()) {
        auto [res, err] = this->RightHandshake(sd);
        sessions[sd] = res;

        if (err != entity::ERR_OK){
            return {"", err};
        }
        secSuite = res;
    }
    else {
        secSuite = it->second;
    }

    //  call raw receive
    auto [res, err] = protocol::RawReceive(sd);
    if(err != entity::ERR_OK) {
        return {"", err};
    }

    //  extract hash and check for integrity
    auto expectedMac = std::vector<uint8_t>(res.end()-sec::MAC_LEN , res.end());
    auto encrypted = std::vector<uint8_t>(res.begin(), res.end()-sec::MAC_LEN);
    //  decrypt using session key

    auto mac = secSuite.mac->MAC(encrypted);
    if(expectedMac != mac) {
        return {
            "",
            entity::ERR_BROKEN
        };    
    }
    
    auto mess = secSuite.sym->decrypt(encrypted);
    
    return {
        std::string(mess.begin(), mess.end()),
        entity::ERR_OK
    };
}

