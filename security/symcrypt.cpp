#include "security.h"
using namespace std;

SymCrypt::SymCrypt(sessionKey k) {
    this->refresh(k);
}

//creates (or updates if already exists) a new key and iv to communicate with userID
void SymCrypt::refresh(sessionKey k) {
    this->key = k;
}

//encrypts pt by using the userID's session key
vector<uint8_t> SymCrypt::encrypt(vector<uint8_t> pt) {
    EVP_CIPHER_CTX *ctx;
    int ctlen, len;
    vector<uint8_t> ct;
    
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        cerr<<"Unable to create a context for SymCrypt"<<endl;
        return {};
    }

    if(EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, this->key.key, this->key.iv) <= 0) {
        cerr<<"Unable to initialize a context for SymCrypt"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    ct.resize(pt.size()+SYMMLEN/8);

    if(EVP_EncryptUpdate(ctx, ct.data(), &len, pt.data(), pt.size()) <= 0) {
        cerr<<"Unable to encrypt with SymCrypt"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    ctlen = len;

    if(EVP_EncryptFinal_ex(ctx, ct.data() + len, &len) <= 0) {
        cerr<<"Unable to encrypt with SymCrypt"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    ctlen += len;

    EVP_CIPHER_CTX_free(ctx);

    return ct;
    
}

//decrypts ct by using the userID's session key
vector<uint8_t> SymCrypt::decrypt(vector<uint8_t> ct) {
    EVP_CIPHER_CTX *ctx;
    int ptlen, len;
    vector<uint8_t> pt;

    if(!(ctx = EVP_CIPHER_CTX_new())) {
        cerr<<"Unable to create a context for SymCrypt"<<endl;
        return {};
    }

    if(EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, this->key.key, this->key.iv) <= 0) {
        cerr<<"Unable to initialize a context for SymCrypt"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    pt.resize(ct.size());

    if(EVP_DecryptUpdate(ctx, pt.data(), &len, ct.data(), ct.size()) < 0) {
        cerr<<"Unable to decrypt with SymCrypt"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    ptlen = len;

    if(EVP_DecryptFinal_ex(ctx, pt.data() + len, &len) < 0) {
        cerr<<"Unable to decrypt with SymCrypt"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    ptlen += len;

    EVP_CIPHER_CTX_free(ctx);

    for(int i = 0; i < pt.size(); i++) {
        if(pt[i] == '\0') {
            pt.resize(i);
            break;
        }
    }

    return pt;
}

SymCrypt::~SymCrypt() {;}