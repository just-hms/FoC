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

    constexpr int SYMMLEN =          256;
    constexpr int SALT_LEN =         16;
    constexpr int HMAC_KEY_LEN =     16;
    constexpr int MAC_LEN =          64;
    constexpr char const* PUBK =     "pubk.pem";
    constexpr char const* PRIVK =    "privk.pem";

    // ASYMCRYPT
    /*EXAMPLE
    AsymCrypt a(private_key_cb, load_public_keys("./clients/"));
    ciphertext c = a.encrypt((unsigned char*)"Lorem ipsum dolor sit amet\n", 0);
    if(c.len != 0) {
        string s = a.decrypt(c);
        cout<<s<<endl;
    }
    */
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


    // SYMCRYPT
    //stores key and iv for AES cipher
    struct sessionKey {
        unsigned char key[SYMMLEN/8], iv[16];
    } typedef sessionKey;


    /*EXAMPLE
    SymCrypt b(0);
    unsigned char *enc = b.encrypt(0, (unsigned char*) "Lorem ipsum dolor sit amet\n");
    cout<<string((char*) b.decrypt(0, enc))<<endl;
    */
    class SymCrypt {
        sessionKey key;

        public:
            SymCrypt();
            SymCrypt(sessionKey sessionKey);
            std::tuple<std::vector<uint8_t>, entity::Error> encrypt(std::vector<uint8_t> plaintext);
            std::tuple<std::vector<uint8_t>, entity::Error> decrypt(std::vector<uint8_t> ciphertext);
    };
    
    entity::Error generateRSAkeys(std::string path, std::string password, unsigned int bits);

    std::tuple<std::vector<uint8_t>, entity::Error> genDHparam(EVP_PKEY *&params);

    // TODO fix memory leak and add error
    std::tuple<EVP_PKEY*, entity::Error> retrieveDHparam(std::vector<uint8_t> DHserialized);
    entity::Error genDH(EVP_PKEY *&dhkey, EVP_PKEY *params);
    std::tuple<std::vector<uint8_t>, entity::Error> derivateDH(EVP_PKEY *your_dhkey, EVP_PKEY *peer_dhkey);
    std::tuple<sessionKey, entity::Error> keyFromSecret(std::vector<uint8_t> secret);

    class Hmac {
        unsigned char key[HMAC_KEY_LEN];

        public:
            Hmac();
            Hmac(std::vector<uint8_t> key);
            std::vector<uint8_t> getKey();
            std::tuple<std::vector<uint8_t>, entity::Error> MAC(std::vector<uint8_t> data);
    };

    std::tuple<std::string, entity::Error> Hash(std::string data);
    std::tuple<std::string, entity::Error> HashAndSalt(std::string password, std::string salt = "");
    bool VerifyHash(std::string hashAndSalt, std::string password);

    std::tuple<std::vector<uint8_t>, entity::Error>encodePeerKey(EVP_PKEY *keyToEncode);
    std::tuple<std::vector<uint8_t>, entity::Error>encodeDH(DH *dh);
    std::tuple<EVP_PKEY*, entity::Error> decodePeerKey(std::vector<uint8_t> encodedKey);

}

#endif