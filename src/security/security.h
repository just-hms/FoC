#ifndef SECURITY_H
#define SECURITY_H

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/dh.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/pem.h>
#include <tuple>
#include "../entity/entity.h"

namespace sec {

    constexpr int SYMMLEN =              256;
    constexpr int SALT_LEN =              16;
    constexpr int HMAC_KEY_LEN =          16;
    constexpr int MAC_LEN =               64;
    constexpr int MAX_SANITIZATION_LEN =  31;
    constexpr char const* PUBK =     "pubk.pem";
    constexpr char const* PRIVK =    "privk.pem";

    class AsymCrypt {
        std::string privk;
        std::string pubk;
        std::string privk_pwd;

        public:
            AsymCrypt(std::string path_private_key, std::string path_public_key_peer, std::string password);
            void setPeerKey(std::string path_public_key_peer);
            std::tuple<std::vector<uint8_t>, entity::Error> encrypt(std::vector<uint8_t> plaintext);
            std::tuple<std::vector<uint8_t>, entity::Error> decrypt(std::vector<uint8_t> ciphertext);
    };

    class SymCrypt {
        unsigned char key[SYMMLEN/8];

        public:
            SymCrypt(std::vector<uint8_t> key);
            std::tuple<std::vector<uint8_t>, entity::Error> encrypt(std::vector<uint8_t> plaintext);
            std::tuple<std::vector<uint8_t>, entity::Error> decrypt(std::vector<uint8_t> ciphertext);
    };

    class Hmac {
        unsigned char key[HMAC_KEY_LEN];

        public:
            Hmac();
            Hmac(std::vector<uint8_t> key);
            std::vector<uint8_t> getKey();
            std::tuple<std::vector<uint8_t>, entity::Error> MAC(std::vector<uint8_t> data);
    };

    //UTILITY FUNCTIONS

    std::tuple<std::string, entity::Error> Hash(std::string data);
    std::tuple<std::string, entity::Error> HashAndSalt(std::string password, std::string salt = "");
    bool VerifyHash(std::string hashAndSalt, std::string password);


    entity::Error generateRSAkeys(std::string path, std::string password, unsigned int bits);
    std::tuple<std::vector<uint8_t>, entity::Error>encodePeerKey(EVP_PKEY *keyToEncode);
    std::tuple<EVP_PKEY*, entity::Error> decodePeerKey(std::vector<uint8_t> encodedKey);

    std::tuple<std::vector<uint8_t>, entity::Error> genDHparam(EVP_PKEY *&params);
    entity::Error genDH(EVP_PKEY *&dhkey, EVP_PKEY *params);
    std::tuple<std::vector<uint8_t>, entity::Error>encodeDH(DH *dh);
    std::tuple<EVP_PKEY*, entity::Error> retrieveDHparam(std::vector<uint8_t> DHserialized);
    std::tuple<std::vector<uint8_t>, entity::Error> derivateDH(EVP_PKEY *your_dhkey, EVP_PKEY *peer_dhkey);
    std::tuple<std::vector<uint8_t>, entity::Error> keyFromSecret(std::vector<uint8_t> secret);

    bool sanitize(std::string, unsigned int);

}

#endif