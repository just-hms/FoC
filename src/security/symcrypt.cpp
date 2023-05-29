#include "security.h"
#include <openssl/err.h>

sec::SymCrypt::SymCrypt(std::vector<uint8_t> k) {
    if(k.size() < SYMMLEN/8) exit(1);
    k.resize(SYMMLEN/8);
    memcpy(&(this->key[0]), k.data(), SYMMLEN/8);
}

//encrypts pt by using the userID's session key
std::tuple<std::vector<uint8_t>, entity::Error> sec::SymCrypt::encrypt(std::vector<uint8_t> pt) {
    EVP_CIPHER_CTX *ctx;
    int ctlen, len;
    std::vector<uint8_t> iv(16);
    std::vector<uint8_t> ct;
    
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        std::cerr<<"Unable to create a context for SymCrypt"<<std::endl;
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }

    if(RAND_bytes(iv.data(), 16) <= 0) {
        std::cerr<<"Unable to create an IV"<<std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }

    if(EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, this->key, iv.data()) <= 0) {
        std::cerr<<"Unable to initialize a context for SymCrypt"<<std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }

    ct.resize(pt.size()+SYMMLEN/8);

    if(EVP_EncryptUpdate(ctx, ct.data(), &len, pt.data(), pt.size()) <= 0) {
        std::cerr<<"Unable to encrypt with SymCrypt"<<std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }
    ctlen = len;

    if(EVP_EncryptFinal_ex(ctx, ct.data() + len, &len) <= 0) {
        std::cerr<<"Unable to encrypt with SymCrypt"<<std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }
    ctlen += len;
    EVP_CIPHER_CTX_free(ctx);
    
    ct.resize(ctlen);
    ct.insert(ct.begin(), iv.begin(), iv.end());

    return {ct, entity::ERR_OK};
}

//decrypts ct by using the userID's session key
std::tuple<std::vector<uint8_t>, entity::Error> sec::SymCrypt::decrypt(std::vector<uint8_t> ct) {
    EVP_CIPHER_CTX *ctx;
    int ptlen, len;
    std::vector<uint8_t> pt, iv;

    iv.insert(iv.end(), ct.begin(), ct.begin() + 16);
    ct.erase(ct.begin(), ct.begin() + 16);

    if(!(ctx = EVP_CIPHER_CTX_new())) {
        std::cerr<<"Unable to create a context for SymCrypt"<<std::endl;
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }

    if(EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, this->key, iv.data()) <= 0) {
        std::cerr<<"Unable to initialize a context for SymCrypt"<<std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }

    pt.resize(ct.size());

    if(EVP_DecryptUpdate(ctx, pt.data(), &len, ct.data(), ct.size()) <= 0) {
        std::cerr<<"Unable to update decrypt with SymCrypt"<<std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }
    ptlen = len;

    if(EVP_DecryptFinal_ex(ctx, pt.data() + len, &len) <= 0) {
        ERR_print_errors_fp(stderr);
        std::cerr<<"Unable to finalize decrypt with SymCrypt"<<std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }
    ptlen += len;
    pt.resize(ptlen);

    EVP_CIPHER_CTX_free(ctx);

    return {pt, entity::ERR_OK};
}