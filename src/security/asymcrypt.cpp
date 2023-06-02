#include "security.h"
#include <cstdint>

namespace sec {

    constexpr int MAX_ENCRYPTION_LEN = 470;

    //loads users's public key contained in path into a hash map and assigns to each user a progressive ID
    AsymCrypt::AsymCrypt(std::string privk_file, std::string pubk_file, std::string pwd) {
        this->privk = privk_file;
        this->pubk = pubk_file;
        this->privk_pwd = pwd;
    }

    void AsymCrypt::setPeerKey(std::string pubk_file) {
        this->pubk = pubk_file;
    }

    //encrypts mes using userID's public key
    std::tuple<std::vector<uint8_t>, entity::Error> AsymCrypt::encrypt(std::vector<uint8_t> mess) {

        EVP_PKEY_CTX *ctx;
        EVP_PKEY *key;
        size_t ctlen;
        std::vector<uint8_t> ct;

        if(this->pubk.size() == 0) {
            std::cerr<<"Peer key hasn't been set yet"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_FILE_NOT_FOUND};
        }

        FILE *fp = fopen((this->pubk).c_str(), "r");
        if(fp == NULL) {
            std::cerr<<"Couldn't open AsymCrypt public key file "<<(this->pubk).c_str() << std::endl;
            return {std::vector<uint8_t>(), entity::ERR_FILE_NOT_FOUND};
        }

        key = PEM_read_PUBKEY(fp, NULL, NULL, NULL);
        if(key == NULL) {
            std::cerr<<"Couldn't read AsymCrypt public key"<<std::endl;
            fclose(fp);
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }
        
        if(!(ctx = EVP_PKEY_CTX_new(key, NULL))) {
            std::cerr<<"Unable to create a contextfor AsymCrypt"<<std::endl;
            EVP_PKEY_free(key);
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        if(EVP_PKEY_encrypt_init(ctx) <= 0) {
            std::cerr<<"Unable to initialize context for AsymCrypt"<<std::endl;
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(key);
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        if(EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
            std::cerr<<"Unable to set padding for AsymCrypt"<<std::endl;
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(key);
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        //determine output buffer length
        if(EVP_PKEY_encrypt(ctx, NULL, &ctlen, mess.data(), mess.size()) <= 0) {
            std::cout<<"Unable to determine ct buffer length"<<std::endl;
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(key);
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }
        
        ct.resize(ctlen);
        
        if(EVP_PKEY_encrypt(ctx, ct.data(), &ctlen, mess.data(), mess.size()) <= 0) {
            std::cerr<<"Error during AsymCrypt encryption"<<std::endl;
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(key);
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);

        return {ct, entity::ERR_OK};
    }

    //decrypts mess using the server's private key
    std::tuple<std::vector<uint8_t>, entity::Error> AsymCrypt::decrypt(std::vector<uint8_t> ct) {
        EVP_PKEY_CTX *ctx;
        EVP_PKEY *key;
        size_t ptlen;
        std::vector<uint8_t> pt;

        FILE *fp = fopen((this->privk).c_str(), "r");
        if(fp == NULL) {
            std::cerr<<"Couldn't open AsymCrypt private key file " << this->privk <<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_FILE_NOT_FOUND};
        }

        key = PEM_read_PrivateKey(fp, NULL, NULL, (void*) this->privk_pwd.data());
        if(key == NULL) {
            std::cerr<<"Couldn't read AsymCrypt private key"<<  std::endl;
            fclose(fp);
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        if(!(ctx = EVP_PKEY_CTX_new(key, NULL))) {
            std::cerr<<"Unable to create a context for AsymCrypt"<<std::endl;
            EVP_PKEY_free(key);
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        if(EVP_PKEY_decrypt_init(ctx) <= 0) {
            std::cerr<<"Unable to initialize context for AsymCrypt"<<std::endl;
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(key);
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        if(EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
            std::cerr<<"Unable to set padding for AsymCrypt"<<std::endl;
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(key);
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }
        if(EVP_PKEY_decrypt(ctx, NULL, &ptlen, ct.data(), ct.size()) <= 0) {
            std::cerr<<"Unable to determine pt buffer length"<<std::endl;
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(key);
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }
        pt.resize(ptlen);

        if(EVP_PKEY_decrypt(ctx, pt.data(), &ptlen, ct.data(), ct.size()) < 0) {
            std::cerr<<"Error during AsymCrypt decryption"<<std::endl;
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(key);
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);

        pt.resize(ptlen);

        return {pt, entity::ERR_OK};
    }
}