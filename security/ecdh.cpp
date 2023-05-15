#include "security.h"
using namespace std;

// Encode the public key as a string
string encodePublicKey(EVP_PKEY* publicKey) {
    BIO* bio = BIO_new(BIO_s_mem());
    if(bio == NULL) {
        cerr<<"Couldn't allocate BIO structure"<<endl;
        BIO_free(bio);
        return "";
    }
    if(PEM_write_bio_PUBKEY(bio, publicKey) <= 0) {
        cerr<<"Couldn't write public key"<<endl;
        BIO_free(bio);
        return "";
    }
    
    char* buffer;
    long length = BIO_get_mem_data(bio, &buffer);
    
    string encodedKey(buffer, length);
    
    BIO_free(bio);
    return encodedKey;
}

EVP_PKEY* decodePublicKey(const string& encodedKey) {
    BIO* bio;
    EVP_PKEY* publicKey;

    bio = BIO_new_mem_buf(encodedKey.c_str(), -1);
    if(!(publicKey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL))) {
        cerr<<"Couldn't read public key"<<endl;
        BIO_free(bio);
        return NULL;
    }

    BIO_free(bio);
    return publicKey;
}

int genDH(EVP_PKEY *&pubk, EVP_PKEY *params) {

    EVP_PKEY_CTX *ctx;

    //generate keys
    if(!(ctx = EVP_PKEY_CTX_new(params, NULL))) {
        cerr<<"Couldn't create a context for DH"<<endl;
        return -1;
    }
    if(EVP_PKEY_keygen_init(ctx) <= 0) {
        cerr<<"Couldn't initialize context for DH"<<endl;
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }
    if(EVP_PKEY_keygen(ctx, &pubk) <= 0) {
        cerr<<"Couldn't generate key for DH"<<endl;
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }
    EVP_PKEY_CTX_free(ctx);

    return 0;

}

//generate g and p
int genECDHparam(EVP_PKEY *&params) {

    DH* tmp = DH_get_2048_256();
    if(tmp == NULL) {
        cerr<<"Couldn't create DH"<<endl;
        return -1;
    }
    if(!(params = EVP_PKEY_new())) {
        cerr<<"Couldn't create DH"<<endl;
        DH_free(tmp);
        return -1;
    }
    if(EVP_PKEY_set1_DH(params, tmp) <= 0) {
        cerr<<"Couldn't set DH"<<endl;
        DH_free(tmp);
        return -1;
    }
    DH_free(tmp);
    
    return 0;
}

string derivateDH(EVP_PKEY *privk, EVP_PKEY *peerk) {
    EVP_PKEY_CTX *ctx;
    if(!(ctx = EVP_PKEY_CTX_new(privk, NULL))) {
        cerr<<"Couldn't create a context for DH"<<endl;
        return "";
    }
    if(EVP_PKEY_derive_init(ctx) <= 0) {
        cerr<<"Couldn't initialize deriving context for DH"<<endl;
        EVP_PKEY_CTX_free(ctx);
        return "";
    }
    if(EVP_PKEY_derive_set_peer(ctx, peerk) <= 0) {
        cerr<<"Couldn't set peer for DH deriving context"<<endl;
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    //derives DH shared secret and returns it in 'secret'
    unsigned char *secret;
    size_t secretlen;

    if(EVP_PKEY_derive(ctx, NULL, &secretlen) <= 0) {
        cerr<<"Couldn't derive DH key length"<<endl;
        EVP_PKEY_CTX_free(ctx);
        return "";
    }
    secret = new unsigned char[secretlen];
    if(EVP_PKEY_derive(ctx, secret, &secretlen) <= 0) {
        cerr<<"Couldn't derive DH key"<<endl;
        EVP_PKEY_CTX_free(ctx);
        return "";
    }
    EVP_PKEY_CTX_free(ctx);

    string res = string((char*) secret);
    delete[] secret;

    return res;
}