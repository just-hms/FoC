#include "security.h"
using namespace std;

//loads users's public key contained in path into a hash map and assigns to each user a progressive ID
AsymCrypt::AsymCrypt(string privk_file, string pubk_file, string pwd) {
    this->privk = privk_file;
    this->pubk = pubk_file;
    this->privk_pwd = pwd;
}

//copies udata into buf and returns the amount of bytes copied
int pem_password_callback(char *buf, int max_len, int flag, void *udata) {
    char* pwd = (char*) udata;
    int len = strnlen(pwd, max_len);

    if(len > max_len)
        return 0;

    memcpy(buf, pwd, len);
    return len;
}

//encrypts mes using userID's public key
vector<uint8_t> AsymCrypt::encrypt(vector<uint8_t> mess) {
    EVP_PKEY_CTX *ctx;
    EVP_PKEY *key;
    size_t ctlen;
    vector<uint8_t> ct;

    FILE *fp = fopen((this->pubk).c_str(), "r");
    if(fp == NULL) {
        cerr<<"Couldn't open AsymCrypt private key file"<<endl;
        return {};
    }
    key = PEM_read_PUBKEY(fp, NULL, NULL, NULL);
    if(key == NULL) {
        cerr<<"Couldn't read AsymCrypt public key"<<endl;
        fclose(fp);
        return {};
    }
    fclose(fp);

    if(!(ctx = EVP_PKEY_CTX_new(key, NULL))) {
        cerr<<"Unable to create a contextfor AsymCrypt"<<endl;
        EVP_PKEY_free(key);
        return {};
    }

    if(EVP_PKEY_encrypt_init(ctx) <= 0) {
        cerr<<"Unable to initialize context for AsymCrypt"<<endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);
        return {};
    }

    if(EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        cerr<<"Unable to set padding for AsymCrypt"<<endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);
        return {};
    }

    //determine output buffer length
    if(EVP_PKEY_encrypt(ctx, NULL, &ctlen, mess.data(), mess.size()) <= 0) {
        cout<<"Unable to determine ct buffer length"<<endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);
        return {};
    }
    ct.resize(ctlen);

    if(EVP_PKEY_encrypt(ctx, ct.data(), &ctlen, mess.data(), mess.size()) <= 0) {
        cerr<<"Error during AsymCrypt encryption"<<endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);
        return {};
    }

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(key);

    return ct;
}

//decrypts mess using the server's private key
vector<uint8_t> AsymCrypt::decrypt(vector<uint8_t> ct) {
    EVP_PKEY_CTX *ctx;
    EVP_PKEY *key;
    size_t ptlen;
    vector<uint8_t> pt;

    FILE *fp = fopen((this->privk).c_str(), "r");
    if(fp == NULL) {
        cerr<<"Couldn't open AsymCrypt private key file"<<endl;
        return {};
    }
    key = PEM_read_PrivateKey(fp, NULL, pem_password_callback, (void*)this->privk_pwd.c_str());
    if(key == NULL) {
        cerr<<"Couldn't read AsymCrypt private key"<<endl;
        fclose(fp);
        return {};
    }
    fclose(fp);

    if(!(ctx = EVP_PKEY_CTX_new(key, NULL))) {
        cerr<<"Unable to create a context for AsymCrypt"<<endl;
        EVP_PKEY_free(key);
        return {};
    }

    if(EVP_PKEY_decrypt_init(ctx) <= 0) {
        cerr<<"Unable to initialize context for AsymCrypt"<<endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);
        return {};
    }

    if(EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        cerr<<"Unable to set padding for AsymCrypt"<<endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);
        return {};
    }
    if(EVP_PKEY_decrypt(ctx, NULL, &ptlen, ct.data(), ct.size()) <= 0) {
        cerr<<"Unable to determine pt buffer length"<<endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);
        return {};
    }
    pt.resize(ptlen);

    if(EVP_PKEY_decrypt(ctx, pt.data(), &ptlen, ct.data(), ct.size()) < 0) {
        cerr<<"Error during AsymCrypt decryption"<<endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);
        return {};
    }

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(key);

    for(int i = 0; i < pt.size(); i++) {
        if(pt[i] == '\0') {
            pt.resize(i);
            break;
        }
    }

    return pt;
}

AsymCrypt::~AsymCrypt() {;}