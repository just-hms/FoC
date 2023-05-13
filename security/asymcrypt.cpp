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
ciphertext AsymCrypt::encrypt(unsigned char* mess) {
    EVP_PKEY_CTX *ctx;
    EVP_PKEY *key;
    ciphertext c;
    size_t inlen = strlen((char*)mess), ctlen;
    unsigned char *ct;

    FILE *fp = fopen((this->pubk).c_str(), "r");
    key = PEM_read_PUBKEY(fp, NULL, NULL, NULL);
    fclose(fp);

    ctx = EVP_PKEY_CTX_new(key, NULL);
    if (!ctx) {
        cerr<<"Unable to create a context for RSA"<<endl;
        return c;
    }
    EVP_PKEY_encrypt_init(ctx);
    EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);

    //determine output buffer length
    if(EVP_PKEY_encrypt(ctx, NULL, &ctlen, mess, inlen) <= 0) {
        cout<<"Unable to determine ct buffer lenght"<<endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);
        return c;
    }
    ct = new unsigned char[ctlen];

    if(EVP_PKEY_encrypt(ctx, ct, &ctlen, mess, inlen) == -1) {
        cerr<<"Error during encryption"<<endl;
        delete[] ct;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);
        return c;
    }

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(key);

    c.body = ct;
    c.len = ctlen;
    return c;
}

//decrypts mess using the server's private key
string AsymCrypt::decrypt(ciphertext ct) {
    EVP_PKEY_CTX *ctx;
    FILE *fp = fopen((this->privk).c_str(), "r");
    if(fp == NULL) {
        cerr<<"Couldn't open private key file"<<endl;
        return NULL;
    }
    EVP_PKEY *key = PEM_read_PrivateKey(fp, NULL, pem_password_callback, (void*)this->privk_pwd.c_str());
    fclose(fp);
    size_t ptlen;
    unsigned char* pt;

    ctx = EVP_PKEY_CTX_new(key, NULL);
    if (!ctx) {
        cerr<<"Unable to create a context for RSA"<<endl;
        return "";
    }

    EVP_PKEY_decrypt_init(ctx);
    EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);

    if(EVP_PKEY_decrypt(ctx, NULL, &ptlen, ct.body, ct.len) <= 0) {
        cerr<<"Unable to determine pt buffer length"<<endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);
        return "";
    }
    pt = new unsigned char[ptlen];

    if(EVP_PKEY_decrypt(ctx, pt, &ptlen, ct.body, ct.len) <= 0) {
        cerr<<"Error during decryption"<<endl;
        delete[] pt;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);
        return "";
    }

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(key);

    return string((char*) pt);
}

AsymCrypt::~AsymCrypt() {;}