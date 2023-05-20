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
            std::vector<uint8_t> encrypt(std::vector<uint8_t> plaintext);
            std::vector<uint8_t> decrypt(std::vector<uint8_t> ciphertext);
    };
    
    int generateRSAkeys(std::string path, std::string password, unsigned int bits);

    std::vector<uint8_t> genDHparam(EVP_PKEY *&params);
    EVP_PKEY* retrieveDHparam(std::vector<uint8_t> DHserialized);
    int genDH(EVP_PKEY *&dhkey, EVP_PKEY *params);
    std::vector<uint8_t> derivateDH(EVP_PKEY *your_dhkey, EVP_PKEY *peer_dhkey);
    sessionKey keyFromSecret(std::vector<uint8_t> secret);

    class Hmac {
        unsigned char key[16];

        public:
            Hmac();
            Hmac(std::vector<uint8_t> key);
            std::vector<uint8_t> getKey();
            std::vector<uint8_t> MAC(std::vector<uint8_t> data);
    };

    std::vector<uint8_t> Hash(std::vector<uint8_t> data);
    std::string Hash(std::string data);
    std::string HashAndSalt(std::string password, std::string salt = "");
    bool VerifyHash(std::string hashAndSalt, std::string password);

    std::string encode(char* data, int datalen);
    std::vector<uint8_t>encodePublicKey(EVP_PKEY *keyToEncode);
    std::vector<uint8_t>encodeDH(DH *dh);
    EVP_PKEY* decodePublicKey(std::vector<uint8_t> encodedKey);

}

#endif