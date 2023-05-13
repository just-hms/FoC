#include "security.h"
using namespace std;

// Encode the public key as a string
std::string encodePublicKey(EVP_PKEY* publicKey) {
    BIO* bio = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(bio, publicKey);
    
    char* buffer;
    long length = BIO_get_mem_data(bio, &buffer);
    
    std::string encodedKey(buffer, length);
    
    BIO_free(bio);
    return encodedKey;
}

EVP_PKEY* decodePublicKey(const std::string& encodedKey) {
    BIO* bio = BIO_new_mem_buf(encodedKey.c_str(), -1);
    EVP_PKEY* publicKey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free(bio);
    return publicKey;
}

void genECDH(EVP_PKEY *&pubk, EVP_PKEY *params) {

    //generate keys
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(params, NULL);
    EVP_PKEY_keygen_init(ctx);
    EVP_PKEY_keygen(ctx, &pubk);
    EVP_PKEY_CTX_free(ctx);

}

//generate g and p
void genECDHparam(EVP_PKEY *&params) {

    DH* tmp = DH_get_2048_256();
    params = EVP_PKEY_new();
    EVP_PKEY_set1_DH(params, tmp);
    DH_free(tmp);

}

string derivateECDH(EVP_PKEY *privk, EVP_PKEY *peerk) {
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(privk, NULL);
    EVP_PKEY_derive_init(ctx);
    EVP_PKEY_derive_set_peer(ctx, peerk);

    //derives DH shared secret and returns it in 'secret'
    unsigned char *secret;
    size_t secretlen;
    EVP_PKEY_derive(ctx, NULL, &secretlen);
    secret = new unsigned char[secretlen];
    EVP_PKEY_derive(ctx, secret, &secretlen);
    EVP_PKEY_CTX_free(ctx);

    return string((char*) secret);
}