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

    FunkyProtocol::FunkyProtocol(FunkyOptions * opt) {
        this->name = opt->name;
        this->peerName = opt->peerName;
        this->dataPath = opt->dataPath;
        this->secret = opt->secret;
    }

    void FunkyProtocol::Disconnect(int sd) {
        this->sessions.erase(sd);
    }

    std::tuple<FunkySecuritySuite,entity::Error> FunkyProtocol::RightHandshake(int sd){

        FunkySecuritySuite suite;

        //  1. Receive handshake message from client, build new AsymCrypt obj 
        auto [message, err] = RawReceive(sd);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        auto asy = sec::AsymCrypt(
            this->dataPath+std::string(this->name +sec::PRIVK), 
            "", 
            this->secret
        );

        asy.setPeerKey(this->dataPath + std::string(message.begin(), message.end()) + sec::PUBK);

        //  2. Generate and send to client DH parameters
        EVP_PKEY *paramsDH, *rightDH;

        std::vector<uint8_t> DH;
        std::tie(DH, err) = sec::genDHparam(paramsDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        defer { EVP_PKEY_free(paramsDH); };

        std::vector<uint8_t> res;
        std::tie(res, err) = asy.sign(DH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        std::vector<uint8_t> signedDH;
        signedDH.insert(signedDH.end(), res.begin(), res.end());
        signedDH.insert(signedDH.end(), DH.begin(), DH.end());

        err = RawSend(sd, signedDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        //  3. Generate and send to client the server's DH public key
        err = sec::genDH(rightDH, paramsDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        defer { EVP_PKEY_free(rightDH); };

        std::vector<uint8_t> encodedRightDH;
        std::tie(encodedRightDH, err) = sec::encodePeerKey(rightDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        
        std::tie(res, err) = asy.sign(encodedRightDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        signedDH.clear();
        signedDH.insert(signedDH.end(), res.begin(), res.end());
        signedDH.insert(signedDH.end(), encodedRightDH.begin(), encodedRightDH.end());

        err = RawSend(sd, signedDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        // 4. Receive client's DH public key
        std::tie(res, err) = RawReceive(sd);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        std::vector<uint8_t> ds_leftDH, encodedLeftDH;
        ds_leftDH.insert(ds_leftDH.end(), res.begin(), res.begin() + 512);
        encodedLeftDH.insert(encodedLeftDH.end(), res.begin() + 512, res.end());
        bool check;
        std::tie(check, err) = asy.verify(encodedLeftDH, ds_leftDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        EVP_PKEY * leftDH;
        std::tie(leftDH, err) = sec::decodePeerKey(encodedLeftDH); 
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        defer {EVP_PKEY_free(leftDH);};

        // 5. Derivate secret, generate key for SymCrypt

        std::vector<uint8_t> secret;
        std::tie(secret, err) = sec::derivateDH(rightDH, leftDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        
        std::vector<uint8_t> keys;
        std::tie(keys, err) = sec::keyFromSecret(secret);
        std::vector<uint8_t> seskey, mackey;
        seskey.insert(seskey.end(), keys.begin(), keys.begin() + sec::SYMMLEN/8);
        mackey.insert(mackey.end(), keys.begin() + sec::SYMMLEN/8, keys.end());
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        suite.sym = std::make_shared<sec::SymCrypt>(sec::SymCrypt(seskey));
        suite.mac = std::make_shared<sec::Hmac>(sec::Hmac(mackey));

        //  6. MAC of the all session
        std::vector<uint8_t> HSsession;
        HSsession.insert(HSsession.end(), message.begin(), message.end());
        HSsession.insert(HSsession.end(), DH.begin(), DH.end());
        HSsession.insert(HSsession.end(), encodedRightDH.begin(), encodedRightDH.end());
        HSsession.insert(HSsession.end(), encodedLeftDH.begin(), encodedLeftDH.end());
        HSsession.insert(HSsession.end(), secret.begin(), secret.end());

        //send mac
        std::vector<uint8_t> hash;
        std::tie(hash, err) = suite.mac->MAC(HSsession);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        std::tie(res, err) = suite.sym->encrypt(hash);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        err = RawSend(sd, res);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        
        //receive mac and check integrity
        std::tie(res, err) = RawReceive(sd);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        std::vector<uint8_t> recvHash;
        std::tie(recvHash, err) = suite.sym->decrypt(res);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        HSsession.insert(HSsession.end(), hash.begin(), hash.end());
        std::tie(hash, err) = suite.mac->MAC(HSsession);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        
        if (recvHash != hash) return {FunkySecuritySuite{}, entity::ERR_DURING_HANDSHAKE};

        return {suite, entity::ERR_OK};

    }


    std::tuple<FunkySecuritySuite,entity::Error> FunkyProtocol::LeftHandshake(int sd){

        FunkySecuritySuite suite;

        auto asy = sec::AsymCrypt(
            this->dataPath+std::string(this->name + sec::PRIVK), 
            this->dataPath+std::string(this->peerName +sec::PUBK), 
            this->secret
        );

        // 1. Send a message with the username
        auto err = RawSend(sd, std::vector<uint8_t>(this->name.begin(), this->name.end()));
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        // 2. Receive DH parameters
        std::vector<uint8_t> res;
        std::tie(res, err) = RawReceive(sd);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        std::vector<uint8_t> ds_DH, DH;
        ds_DH.insert(ds_DH.end(), res.begin(), res.begin() + 512);
        DH.insert(DH.end(), res.begin() + 512, res.end());
        bool check;
        std::tie(check, err) = asy.verify(DH, ds_DH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        EVP_PKEY *paramsDH;
        std::tie(paramsDH, err) = sec::retrieveDHparam(DH);  
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        defer {EVP_PKEY_free(paramsDH);};

        // 3. Receive rightDH key
        std::tie(res, err) = RawReceive(sd);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        std::vector<uint8_t> encodedRightDH;
        ds_DH.clear();
        ds_DH.insert(ds_DH.end(), res.begin(), res.begin() + 512);
        encodedRightDH.insert(encodedRightDH.end(), res.begin() + 512, res.end());
        std::tie(check, err) = asy.verify(encodedRightDH, ds_DH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        EVP_PKEY *rightDH;
        std::tie(rightDH, err) = sec::decodePeerKey(encodedRightDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        defer { EVP_PKEY_free(rightDH); };

        // 4. Send leftDH key
        EVP_PKEY *leftDH;
        err = sec::genDH(leftDH, paramsDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        defer {EVP_PKEY_free(leftDH);};
        
        std::vector<uint8_t> encodedLeftDH;
        std::tie(encodedLeftDH, err) = sec::encodePeerKey(leftDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        std::tie(res, err) = asy.sign(encodedLeftDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        std::vector<uint8_t> signedDH;
        signedDH.insert(signedDH.end(), res.begin(), res.end());
        signedDH.insert(signedDH.end(), encodedLeftDH.begin(), encodedLeftDH.end());

        err = RawSend(sd, signedDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        // 5. Derivate secret, generate key for SymCrypt

        std::vector<uint8_t> secret;
        std::tie(secret, err) = sec::derivateDH(leftDH, rightDH);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        std::vector<uint8_t> keys;
        std::tie(keys, err) = sec::keyFromSecret(secret);
        std::vector<uint8_t> seskey, mackey;
        seskey.insert(seskey.end(), keys.begin(), keys.begin() + sec::SYMMLEN/8);
        mackey.insert(mackey.end(), keys.begin() + sec::SYMMLEN/8, keys.end());
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        suite.sym = std::make_shared<sec::SymCrypt>(sec::SymCrypt(seskey));
        suite.mac = std::make_shared<sec::Hmac>(sec::Hmac(mackey));

        //  6. MAC of all the session
        std::vector<uint8_t> HSsession;
        HSsession.insert(HSsession.end(), this->name.begin(), this->name.end());
        HSsession.insert(HSsession.end(), DH.begin(), DH.end());
        HSsession.insert(HSsession.end(), encodedRightDH.begin(), encodedRightDH.end());
        HSsession.insert(HSsession.end(), encodedLeftDH.begin(), encodedLeftDH.end());
        HSsession.insert(HSsession.end(), secret.begin(), secret.end());

        //receive mac and check integrity
        std::tie(res, err) = RawReceive(sd);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        std::vector<uint8_t> recvHash;
        std::tie(recvHash, err) = suite.sym->decrypt(res);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};

        std::vector<uint8_t> hash;
        std::tie(hash, err) = suite.mac->MAC(HSsession);
        if (err != entity::ERR_OK) return {FunkySecuritySuite{}, err};
        
        if (recvHash != hash) return {FunkySecuritySuite{}, entity::ERR_DURING_HANDSHAKE};

        //send mac
        HSsession.insert(HSsession.end(), recvHash.begin(), recvHash.end());
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
        std::vector<uint8_t> tmp;
        auto uintTS = secSuite.sym->incrementCounter(0);
        if(uintTS == 0) return entity::ERR_BROKEN;
        auto ts = std::to_string(uintTS);
        ts.insert(ts.begin(), sec::CTR_LEN - ts.size(), '0');
        std::vector<uint8_t> timestamp = std::vector<uint8_t>(ts.begin(), ts.end());
        res.insert(res.begin(), timestamp.begin(), timestamp.end());
        
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
        if(res.size() < sec::MAC_LEN) return {"", err};
        auto expectedMac = std::vector<uint8_t>(res.end()-sec::MAC_LEN , res.end());
        auto encrypted = std::vector<uint8_t>(res.begin(), res.end()-sec::MAC_LEN);
        //  decrypt using session key
        
        std::vector<uint8_t> mac;
        std::tie(mac, err) = secSuite.mac->MAC(encrypted);
        if(expectedMac != mac) return {"", entity::ERR_BROKEN};

        std::vector<uint8_t> ct, counter;
        if(encrypted.size() < sec::CTR_LEN) return {"", err};
        counter.insert(counter.end(), encrypted.begin(), encrypted.begin() + sec::CTR_LEN);
        ct.insert(ct.end(), encrypted.begin() + sec::CTR_LEN, encrypted.end());

        auto counterINT = stoul(std::string(counter.begin(), counter.end()));
        if(counterINT == 0) return {"", entity::ERR_BROKEN};
        if(counterINT - 1 != secSuite.sym->getCounter(1)) return {"", entity::ERR_BROKEN};
        secSuite.sym->incrementCounter(1);
        
        std::tie(res, err) = secSuite.sym->decrypt(ct);
        if(err != entity::ERR_OK) return {"", err};
        
        return {
            std::string(res.begin(), res.end()),
            entity::ERR_OK
        };
    }

}