#include "security.h"

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
    std::tuple<std::vector<uint8_t>, entity::Error> AsymCrypt::sign(std::vector<uint8_t> mess) {

        FILE *fp = fopen((this->privk).c_str(), "r");
        if(fp == NULL) {
            std::cerr<<"Couldn't open AsymCrypt private key file " << this->privk <<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_FILE_NOT_FOUND};
        }
        defer { fclose(fp); };

        EVP_PKEY * key = PEM_read_PrivateKey(fp, NULL, NULL, (void*) this->privk_pwd.data());
        if(key == NULL) {
            std::cerr<<"Couldn't read AsymCrypt private key"<<  std::endl;
            fclose(fp);
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }
        defer { EVP_PKEY_free(key); };

        EVP_MD_CTX *ctx;
        if(!(ctx = EVP_MD_CTX_new())) {
            std::cerr<<"Unable to create a context to sign"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }
        defer {EVP_MD_CTX_free(ctx);};

        if(EVP_SignInit(ctx, EVP_sha256()) <= 0) {
            std::cerr<<"Unable to initialize context to sign"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        if(EVP_SignUpdate(ctx, mess.data(), mess.size()) <= 0) {
            std::cout<<"Unable to update digital signature"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }
        
        unsigned int sign_len = EVP_PKEY_size(key);
        std::vector<uint8_t> signed_msg(sign_len);
        if (EVP_SignFinal(ctx, signed_msg.data(), &sign_len, key) <= 0) {
            std::cout<<"Unable to finalize digital signature"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }
        signed_msg.resize(sign_len);

        return {signed_msg, entity::ERR_OK};
    }

    //decrypts mess using the server's private key
    std::tuple<bool, entity::Error> AsymCrypt::verify(std::vector<uint8_t> msg, std::vector<uint8_t> signature) {

        if(this->pubk.size() == 0) {
            std::cerr<<"Peer key hasn't been set yet"<<std::endl;
            return {false, entity::ERR_FILE_NOT_FOUND};
        }

        FILE *fp = fopen((this->pubk).c_str(), "r");
        if(fp == NULL) {
            std::cerr<<"Couldn't open AsymCrypt public key file "<<(this->pubk).c_str() << std::endl;
            return {false, entity::ERR_FILE_NOT_FOUND};
        }
        defer { fclose(fp); };

        EVP_PKEY *key = PEM_read_PUBKEY(fp, NULL, NULL, NULL);
        if(key == NULL) {
            std::cerr<<"Couldn't read AsymCrypt public key"<<std::endl;
            return {false, entity::ERR_BROKEN};
        }
        defer { EVP_PKEY_free(key); };

        EVP_MD_CTX *ctx;
        if(!(ctx = EVP_MD_CTX_new())) {
            std::cerr<<"Unable to create a context to sign"<<std::endl;
            return {false, entity::ERR_BROKEN};
        }
        defer {EVP_MD_CTX_free(ctx);};

        if(EVP_VerifyInit(ctx, EVP_sha256()) <= 0) {
            std::cerr<<"Unable to initialize context to sign"<<std::endl;
            return {false, entity::ERR_BROKEN};
        }

        if(EVP_VerifyUpdate(ctx, msg.data(), msg.size()) <= 0) {
            std::cerr<<"Unable to verify"<<std::endl;
            return {false, entity::ERR_BROKEN};
        }

        if(EVP_VerifyFinal(ctx, signature.data(), signature.size(), key) == 1) {
            return {true, entity::ERR_OK};
        }
        else {
            std::cerr<<"Digital signature verification failed"<<std::endl;
            return {false, entity::ERR_BROKEN};
        }
    }
}