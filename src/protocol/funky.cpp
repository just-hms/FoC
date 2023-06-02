#include "protocol.h"
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <memory>
#include <openssl/types.h>
#include <span>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>

namespace protocol {

    FunkyProtocol::FunkyProtocol(FunkyOptions * opt){
        this->name = opt->name;
        this->peerName = opt->peerName;
        this->dataPath = opt->dataPath;
        this->secret = opt->secret;
    }

    void FunkyProtocol::Disconnect(int sd){
        this->sessions.erase(sd);
    }

    std::tuple<FunkySecuritySuite,entity::Error> FunkyProtocol::RightHandshake(int sd){

        FunkySecuritySuite suite;

        //  1. Receive handshake message from client, build new AsymCrypt obj 
        auto [res, err] = RawReceive(sd);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        auto asy = sec::AsymCrypt(
            this->dataPath+std::string(this->name +sec::PRIVK), 
            "", 
            this->secret
        );

        std::vector<uint8_t> message;
        std::tie(message, err) = asy.decrypt(res);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};  

        char buffer[sec::MAX_SANITIZATION_LEN];
        size_t timestamp;

        sscanf((char*) message.data(), "%30s %zu\n", &buffer, &timestamp);

        //message has to be sent within the ACCEPTANCE_WINDOW
        auto ts = time(NULL);
        if(!(timestamp >= ts - entity::ACCEPTANCE_WINDOW && timestamp <= ts + entity::ACCEPTANCE_WINDOW)) {
            return {FunkySecuritySuite{}, entity::ERR_BROKEN};  
        }

        asy.setPeerKey(this->dataPath + std::string(buffer) + sec::PUBK);

        //  2. Generate and send to client a temporary symmetric key
        std::vector<uint8_t> tmpKey(sec::SYMMLEN/8);
        RAND_bytes(tmpKey.data(), sec::SYMMLEN/8);
        sec::SymCrypt symTMP(tmpKey);
        std::vector<uint8_t> aesTMP(sec::SYMMLEN/8);
        memcpy(aesTMP.data(), tmpKey.data(), sec::SYMMLEN/8);

        std::tie(res, err) = asy.encrypt(aesTMP);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};  

        err = RawSend(sd, res);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        //  3. Generate and send to client DH parameters
        EVP_PKEY *paramsDH, *rightDH;

        std::vector<uint8_t> DH;
        std::tie(DH, err) = sec::genDHparam(paramsDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        
        std::tie(res, err) = symTMP.encrypt(DH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        err = RawSend(sd, res);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        //  4. Generate and send to client the server's DH public key
        sec::genDH(rightDH, paramsDH);

        std::vector<uint8_t> encodedRightDH;
        std::tie(encodedRightDH, err) = sec::encodePeerKey(rightDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        
        
        std::tie(res, err) = symTMP.encrypt(encodedRightDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        err = RawSend(sd, res);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        // 5. Receive client's DH public key
        std::tie(res, err) = RawReceive(sd);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        
        std::vector<uint8_t> encodedLeftDH;
        std::tie(encodedLeftDH, err)  = symTMP.decrypt(res);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        EVP_PKEY * leftDH;
        std::tie(leftDH, err) = sec::decodePeerKey(encodedLeftDH); 
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        // 6. Derivate secret, generate key for SymCrypt

        std::vector<uint8_t> secret;
        std::tie(secret, err) = sec::derivateDH(rightDH, leftDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        
        std::vector<uint8_t> key;
        std::tie(key, err) = sec::keyFromSecret(secret);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        suite.sym = std::make_shared<sec::SymCrypt>(sec::SymCrypt(key));

        //  7. Creation and delivery of MAC symmetric key
        suite.mac = std::make_shared<sec::Hmac>(sec::Hmac());

        auto MACk = suite.mac->getKey();

        std::tie(res, err) = suite.sym->encrypt(MACk);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        err = RawSend(sd, res);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        //  8. MAC of the all session
        
        std::vector<uint8_t> HSsession;
        HSsession.insert(HSsession.end(), message.begin(), message.end());
        HSsession.insert(HSsession.end(), DH.begin(), DH.end());
        HSsession.insert(HSsession.end(), encodedRightDH.begin(), encodedRightDH.end());
        HSsession.insert(HSsession.end(), encodedLeftDH.begin(), encodedLeftDH.end());
        HSsession.insert(HSsession.end(), secret.begin(), secret.end());

        std::vector<uint8_t> expectedHash;
        std::tie(expectedHash, err) = suite.mac->MAC(HSsession);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        
        std::tie(res, err) = RawReceive(sd);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        std::vector<uint8_t> hash;
        std::tie(hash, err) = suite.sym->decrypt(res);

        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        
        if(hash != expectedHash) return {FunkySecuritySuite{}, entity::ERR_DURING_HANDSHAKE};

        return {suite, entity::ERR_OK};

    exit_with_err:
        EVP_PKEY_free(paramsDH);
        EVP_PKEY_free(rightDH);
        EVP_PKEY_free(leftDH);
        return {FunkySecuritySuite{}, err};
    }


    std::tuple<FunkySecuritySuite,entity::Error> FunkyProtocol::LeftHandshake(int sd){

        FunkySecuritySuite suite;

        auto asy = sec::AsymCrypt(
            this->dataPath+std::string(this->name + sec::PRIVK), 
            this->dataPath+std::string(this->peerName +sec::PUBK), 
            this->secret
        );

        // 1. Send a message with the username and the current time encryted with RSA
        auto now = time(NULL);
        auto message = this->name + " " + std::to_string(now);

        auto [res, err] = asy.encrypt(
            std::vector<uint8_t>(message.begin(), message.end())
        );
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        err = RawSend(sd, res);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        // 2. Receive temporary symmetric key
        std::tie(res, err) = RawReceive(sd);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        std::vector<uint8_t> encodedSK;

        std::tie(encodedSK, err) = asy.decrypt(res);
        std::vector<uint8_t> tmpKey(sec::SYMMLEN/8);
        memcpy(tmpKey.data(), encodedSK.data(), sec::SYMMLEN/8);
        sec::SymCrypt symTMP(tmpKey);

        // 3. Receive DH parameters
        std::tie(res, err) = RawReceive(sd);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        std::vector<uint8_t> DH;
        std::tie(DH, err) = symTMP.decrypt(res);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        EVP_PKEY *paramsDH;
        std::tie(paramsDH, err) = sec::retrieveDHparam(DH);  
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        // 4. Receive rightDH key
        std::tie(res, err) = RawReceive(sd);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        std::vector<uint8_t> encodedRightDH;
        std::tie(encodedRightDH, err) = symTMP.decrypt(res);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        // 5. Send leftDH key
        EVP_PKEY *leftDH;
        
        sec::genDH(leftDH, paramsDH);

        std::vector<uint8_t> encodedLeftDH;

        std::tie(encodedLeftDH, err) = sec::encodePeerKey(leftDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        EVP_PKEY * rightDH;
        std::tie(rightDH, err) = sec::decodePeerKey(encodedRightDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};


        std::tie(res, err) = symTMP.encrypt(encodedLeftDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};


        err = RawSend(sd, res);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        // 6. Derivate secret, generate key for SymCrypt

        std::vector<uint8_t> secret;
        std::tie(secret, err) = sec::derivateDH(leftDH, rightDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        EVP_PKEY_free(paramsDH);
        EVP_PKEY_free(rightDH);
        EVP_PKEY_free(leftDH);

        std::vector<uint8_t> key;
        std::tie(key, err) = sec::keyFromSecret(secret);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        suite.sym = std::make_shared<sec::SymCrypt>(sec::SymCrypt(key));

        // 7. Receive the key to be used in MAC 
        std::tie(res, err) = RawReceive(sd);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        

        std::vector<uint8_t>decryptedMacKey;  
        std::tie(decryptedMacKey, err) = suite.sym->decrypt(res);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        
        suite.mac = std::make_shared<sec::Hmac>(sec::Hmac(decryptedMacKey));

        //  8. MAC of all the session
        std::vector<uint8_t> HSsession;
        HSsession.insert(HSsession.end(), message.begin(), message.end());
        HSsession.insert(HSsession.end(), DH.begin(), DH.end());
        HSsession.insert(HSsession.end(), encodedRightDH.begin(), encodedRightDH.end());
        HSsession.insert(HSsession.end(), encodedLeftDH.begin(), encodedLeftDH.end());
        HSsession.insert(HSsession.end(), secret.begin(), secret.end());

        std::vector<uint8_t> hash;
        std::tie(hash, err) = suite.mac->MAC(HSsession);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        std::tie(res, err) = suite.sym->encrypt(hash);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        
        err = RawSend(sd, res);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        return {suite, entity::ERR_OK};
    }

    entity::Error FunkyProtocol::Send(int sd, std::string message){
        
        FunkySecuritySuite secSuite;
        
        auto it = this->sessions.find(sd);
        // if exists set it
        if(it == this->sessions.end()) {
            auto [res, err] = this->LeftHandshake(sd);
            if (err != entity::ERR_OK){
                return err;
            }
            sessions[sd] = res;
            secSuite = res;
        }
        else secSuite = it->second;

        //  encrypt using session key
        auto [res, err] = secSuite.sym->encrypt(
            std::vector<uint8_t>(message.begin(), message.end())
        );
        if (err != entity::ERR_OK) return err;

        // generating hash for integrity
        std::vector<uint8_t> mac;

        std::tie(mac, err) = secSuite.mac->MAC(res);
        if (err != entity::ERR_OK) return err;

        // add hash to the message
        res.insert(res.end(), mac.begin(), mac.end());
        // call rawsend
        return RawSend(sd, res);
    }

    std::tuple<std::string,entity::Error> FunkyProtocol::Receive(int sd){

        FunkySecuritySuite secSuite;
        
        auto it = this->sessions.find(sd);
        // if exists set it
        if(it == this->sessions.end()) {
            auto [res, err] = this->RightHandshake(sd);
            if (err != entity::ERR_OK){
                return {"", err};
            }
            sessions[sd] = res;

            secSuite = res;
        }
        else secSuite = it->second;

        //  call raw receive
        auto [res, err] = RawReceive(sd);
        if(err != entity::ERR_OK) return {"", err};

        //  extract hash and check for integrity
        auto expectedMac = std::vector<uint8_t>(res.end()-sec::MAC_LEN , res.end());
        auto encrypted = std::vector<uint8_t>(res.begin(), res.end()-sec::MAC_LEN);
        //  decrypt using session key
        
        std::vector<uint8_t> mac;
        std::tie(mac, err) = secSuite.mac->MAC(encrypted);
        
        if(expectedMac != mac) return {"", entity::ERR_BROKEN};
        
        std::tie(res, err) = secSuite.sym->decrypt(encrypted);
        if(err != entity::ERR_OK) return {"", err};
        
        return {
            std::string(res.begin(), res.end()),
            entity::ERR_OK
        };
    }

}