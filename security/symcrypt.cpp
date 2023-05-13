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
unsigned char* SymCrypt::encrypt(unsigned char *pt) {
    EVP_CIPHER_CTX *ctx;
    int ptlen = strlen((char*)pt)+1, ctlen, len;
    unsigned char *ct;
    
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        cerr<<"Unable to create a context for AES"<<endl;
        return NULL;
    }

    if(EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, this->key.key, this->key.iv) <= 0) {
        cerr<<"Unable to initialize a context for AES"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    ct = new unsigned char[ptlen+16];

    if(EVP_EncryptUpdate(ctx, ct, &len, pt, ptlen) <= 0) {
        cerr<<"Unable to encrypt with AES"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        delete[] ct;
        return NULL;
    }
    ctlen = len;

    if(EVP_EncryptFinal_ex(ctx, ct + len, &len) <= 0)
    ctlen += len;

    EVP_CIPHER_CTX_free(ctx);
    return ct;
    
}

//decrypts ct by using the userID's session key
unsigned char* SymCrypt::decrypt(unsigned char *ct) {
    EVP_CIPHER_CTX *ctx;
    int ctlen = strlen((char*)ct)+1, ptlen, len;
    unsigned char *pt;

    if(!(ctx = EVP_CIPHER_CTX_new())) {
        cerr<<"Unable to create a context for AES"<<endl;
        return NULL;
    }

    if(EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, this->key.key, this->key.iv) <= 0) {
        cerr<<"Unable to initialize a context for AES"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    pt = new unsigned char[ctlen];

    if(EVP_DecryptUpdate(ctx, pt, &len, ct, ctlen) <= 0) {
        cerr<<"Unable to decrypt with AES"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        delete[] pt;
        return NULL;
    }
    ptlen = len;

    if(EVP_DecryptFinal_ex(ctx, pt + len, &len) <= 0)
    ptlen += len;

    EVP_CIPHER_CTX_free(ctx);
    return pt;
}

SymCrypt::~SymCrypt() {;}