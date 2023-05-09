#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/pem.h>

using namespace std;

#define PUBK "pubk.pem"
#define PRIVK "privk.pem"

struct ciphertext {
    unsigned char *body;
    size_t len = 0;

} typedef ciphertext;

class AsymCrypt {
    function<EVP_PKEY* ()> privk_cb;
    unordered_map<uint8_t, EVP_PKEY*> public_keys;

    public:
        AsymCrypt(function<EVP_PKEY* ()>, unordered_map<uint8_t, EVP_PKEY*>);
        ciphertext encrypt(unsigned char*, uint8_t);
        string decrypt(ciphertext);
        ~AsymCrypt();
};

//loads users's public key contained in path into a hash map and assigns to each user a progressive ID
AsymCrypt::AsymCrypt(function<EVP_PKEY* ()> callback, unordered_map<uint8_t, EVP_PKEY*> keys) {
    this->privk_cb = callback;
    this->public_keys = keys;
}

//encrypts mes using userID's public key
ciphertext AsymCrypt::encrypt(unsigned char* mess, uint8_t destID) {
    EVP_PKEY_CTX *ctx;
    EVP_PKEY *key;
    ciphertext c;
    size_t inlen = strlen((char*)mess), ctlen;
    unsigned char *ct;

    key = this->public_keys[destID];

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
    EVP_PKEY *key = this->privk_cb();
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

AsymCrypt::~AsymCrypt() {
    for(auto it = this->public_keys.begin(); it != this->public_keys.end(); it++)
        EVP_PKEY_free(it->second);
    this->public_keys.clear();
}

/*EXAMPLE
//RSA
generateRSAkeys("user1", "pwd1", 4096);
AsymCrypt a(private_key_cb, load_public_keys("./clients/"));
ciphertext c = a.encrypt((unsigned char*)"Lorem ipsum dolor sit amet\n", 0);
if(c.len != 0) {
    string s = a.decrypt(c);
    cout<<s<<endl;
}
*/