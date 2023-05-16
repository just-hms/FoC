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

namespace sec {

    #define SYMMLEN 256
    #define SALT_LEN 16
    #define HMAC_KEY_LEN 16
    #define PUBK "pubk.pem"
    #define PRIVK "privk.pem"

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
            AsymCrypt(std::string, std::string, std::string);
            std::vector<uint8_t> encrypt(std::vector<uint8_t>);
            std::vector<uint8_t> decrypt(std::vector<uint8_t>);
            ~AsymCrypt();
    };


    // SYMCRYPT
    //stores key and iv for AES cipher
    struct sessionKey {
        unsigned char key[SYMMLEN/8], iv[SYMMLEN/8];
    } typedef sessionKey;


    /*EXAMPLE
    SymCrypt b(0);
    unsigned char *enc = b.encrypt(0, (unsigned char*) "Lorem ipsum dolor sit amet\n");
    cout<<string((char*) b.decrypt(0, enc))<<endl;
    */
    class SymCrypt {
        sessionKey key;

        public:
            SymCrypt(sessionKey);
            void refresh(sessionKey);
            std::vector<uint8_t> encrypt(std::vector<uint8_t>);
            std::vector<uint8_t> decrypt(std::vector<uint8_t>);
            ~SymCrypt();
    };

    // RSAGEN
    int generateRSAkeys(std::string, std::string, unsigned int);

    //DH
    int genDHparam(EVP_PKEY*&);
    int genDH(EVP_PKEY*&, EVP_PKEY*);
    std::vector<uint8_t> derivateDH(EVP_PKEY*, EVP_PKEY*);
    sessionKey keyFromSecret(std::string);

    //HMAC
    class Hmac {
        unsigned char key[16];

        public:
            Hmac();
            Hmac(std::vector<uint8_t>);
            std::vector<uint8_t> MAC(std::vector<uint8_t>);
    };

    // PASSWORD
    std::string Hash(std::string);
    std::string HashAndSalt(std::string password, std::string salt = "");
    bool VerifyHash(std::string, std::string);

    //ENCODING
    std::string encode(char*, int);
}

#endif