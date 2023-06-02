#include "security.h"
#include <openssl/err.h>

namespace sec {

    SymCrypt::SymCrypt(std::vector<uint8_t> k) {
        if(k.size() < SYMMLEN/8) exit(1);
        k.resize(SYMMLEN/8);
        memcpy(&(this->key[0]), k.data(), SYMMLEN/8);
    }

    //encrypts pt by using the userID's session key
    std::tuple<std::vector<uint8_t>, entity::Error> SymCrypt::encrypt(std::vector<uint8_t> pt) {
        
        EVP_CIPHER_CTX *ctx;
        if(!(ctx = EVP_CIPHER_CTX_new())) {
            std::cerr<<"Unable to create a context for SymCrypt"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }
        defer {EVP_CIPHER_CTX_free(ctx);};

        std::vector<uint8_t> iv(16);
        if(RAND_bytes(iv.data(), iv.size()) <= 0) {
            std::cerr<<"Unable to create an IV"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        if(EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, this->key, iv.data()) <= 0) {
            std::cerr<<"Unable to initialize a context for SymCrypt"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        std::vector<uint8_t> ct(pt.size()+SYMMLEN/8);
        int ctlen, len;
        if(EVP_EncryptUpdate(ctx, ct.data(), &len, pt.data(), pt.size()) <= 0) {
            std::cerr<<"Unable to encrypt with SymCrypt"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }
        ctlen = len;

        if(EVP_EncryptFinal_ex(ctx, ct.data() + len, &len) <= 0) {
            std::cerr<<"Unable to encrypt with SymCrypt"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }
        ctlen += len;
        
        ct.resize(ctlen);
        ct.insert(ct.begin(), iv.begin(), iv.end());

        return {ct, entity::ERR_OK};
    }

    //decrypts ct by using the userID's session key
    std::tuple<std::vector<uint8_t>, entity::Error> SymCrypt::decrypt(std::vector<uint8_t> ct) {
        std::vector<uint8_t> iv;

        iv.insert(iv.end(), ct.begin(), ct.begin() + 16);
        ct.erase(ct.begin(), ct.begin() + 16);

        EVP_CIPHER_CTX *ctx;
        if(!(ctx = EVP_CIPHER_CTX_new())) {
            std::cerr<<"Unable to create a context for SymCrypt"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }
        defer {EVP_CIPHER_CTX_free(ctx);};

        if(EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, this->key, iv.data()) <= 0) {
            std::cerr<<"Unable to initialize a context for SymCrypt"<<std::endl;
            
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        std::vector<uint8_t> pt(ct.size());
        int ptlen, len;

        if(EVP_DecryptUpdate(ctx, pt.data(), &len, ct.data(), ct.size()) <= 0) {
            std::cerr<<"Unable to update decrypt with SymCrypt"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }
        ptlen = len;

        if(EVP_DecryptFinal_ex(ctx, pt.data() + len, &len) <= 0) {
            ERR_print_errors_fp(stderr);
            std::cerr<<"Unable to finalize decrypt with SymCrypt"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }
        ptlen += len;
        pt.resize(ptlen);

        return {pt, entity::ERR_OK};
    }
}