#include "security.h"

sec::SymCrypt::SymCrypt() {
    RAND_bytes(this->key.key, SYMMLEN/8);
    RAND_bytes(this->key.iv, 16);
}

sec::SymCrypt::SymCrypt(sec::sessionKey k) {
    this->key = k;
}

//encrypts pt by using the userID's session key
std::vector<uint8_t> sec::SymCrypt::encrypt(std::vector<uint8_t> pt) {
    EVP_CIPHER_CTX *ctx;
    int ctlen, len;
    std::vector<uint8_t> ct;
    
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        std::cerr<<"Unable to create a context for SymCrypt"<<std::endl;
        return {};
    }

    if(EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, this->key.key, this->key.iv) <= 0) {
        std::cerr<<"Unable to initialize a context for SymCrypt"<<std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    ct.resize(pt.size()+SYMMLEN/8);

    if(EVP_EncryptUpdate(ctx, ct.data(), &len, pt.data(), pt.size()) <= 0) {
        std::cerr<<"Unable to encrypt with SymCrypt"<<std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    ctlen = len;

    if(EVP_EncryptFinal_ex(ctx, ct.data() + len, &len) <= 0) {
        std::cerr<<"Unable to encrypt with SymCrypt"<<std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    ctlen += len;
    EVP_CIPHER_CTX_free(ctx);

    ct.resize(ctlen);

    return ct;
    
}

//decrypts ct by using the userID's session key
std::vector<uint8_t> sec::SymCrypt::decrypt(std::vector<uint8_t> ct) {
    EVP_CIPHER_CTX *ctx;
    int ptlen, len;
    std::vector<uint8_t> pt;

    if(!(ctx = EVP_CIPHER_CTX_new())) {
        std::cerr<<"Unable to create a context for SymCrypt"<<std::endl;
        return {};
    }

    if(EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, this->key.key, this->key.iv) <= 0) {
        std::cerr<<"Unable to initialize a context for SymCrypt"<<std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    pt.resize(ct.size());

    if(EVP_DecryptUpdate(ctx, pt.data(), &len, ct.data(), ct.size()) <= 0) {
        std::cerr<<"Unable to update decrypt with SymCrypt"<<std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    ptlen = len;

    if(EVP_DecryptFinal_ex(ctx, pt.data() + len, &len) <= 0) {
        std::cerr<<"Unable to finalize decrypt with SymCrypt"<<std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    ptlen += len;

    pt.resize(ptlen);

    EVP_CIPHER_CTX_free(ctx);

    return pt;
}