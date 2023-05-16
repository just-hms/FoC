#include "security.h"

// Encode the public key as a string
std::vector<uint8_t> sec::encodePublicKey(EVP_PKEY* publicKey) {

    BIO* bio = BIO_new(BIO_s_mem());
    std::vector<uint8_t>res;
    if(bio == NULL) {
        std::cerr<<"Couldn't allocate BIO structure"<<std::endl;
        BIO_free(bio);
        return {};
    }
    if(PEM_write_bio_PUBKEY(bio, publicKey) <= 0) {
        std::cerr<<"Couldn't write public key"<<std::endl;
        BIO_free(bio);
        return {};
    }
    char *buffer;
    res.resize(BIO_get_mem_data(bio, &buffer));
    memcpy(res.data(), buffer, res.size());
    delete buffer;
    
    BIO_free(bio);
    return res;
}

EVP_PKEY* sec::decodePublicKey(std::vector<uint8_t> encodedKey) {
    
    BIO* bio;
    EVP_PKEY* publicKey;
    bio = BIO_new_mem_buf(encodedKey.data(), encodedKey.size());
    if(!(publicKey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL))) {
        std::cerr<<"Couldn't read public key"<<std::endl;
        BIO_free(bio);
        return NULL;
    }

    BIO_free(bio);
    return publicKey;
}

//generate g and p
int sec::genDHparam(EVP_PKEY *&params) {

    DH* tmp = DH_get_2048_256();
    if(tmp == NULL) {
        std::cerr<<"Couldn't create DH"<<std::endl;
        return -1;
    }

    if(!(params = EVP_PKEY_new())) {
        std::cerr<<"Couldn't create DH"<<std::endl;
        DH_free(tmp);
        return -1;
    }

    if(EVP_PKEY_set1_DH(params, tmp) <= 0) {
        std::cerr<<"Couldn't set DH"<<std::endl;
        DH_free(tmp);
        return -1;
    }

    DH_free(tmp);
    
    return 0;
}

int sec::genDH(EVP_PKEY *&pubk, EVP_PKEY *params) {

    EVP_PKEY_CTX *ctx;

    //generate keys
    if(!(ctx = EVP_PKEY_CTX_new(params, NULL))) {
        std::cerr<<"Couldn't create a context for DH"<<std::endl;
        return -1;
    }

    if(EVP_PKEY_keygen_init(ctx) <= 0) {
        std::cerr<<"Couldn't initialize context for DH"<<std::endl;
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    pubk = NULL;
    if(EVP_PKEY_keygen(ctx, &pubk) <= 0) {
        std::cerr<<"Couldn't generate key for DH"<<std::endl;
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    EVP_PKEY_CTX_free(ctx);

    return 0;

}

std::vector<uint8_t> sec::derivateDH(EVP_PKEY *privk, EVP_PKEY *peerk) {
    EVP_PKEY_CTX *ctx;
    std::vector<uint8_t> secret;
    size_t secretlen;

    if(!(ctx = EVP_PKEY_CTX_new(privk, NULL))) {
        std::cerr<<"Couldn't create a context for DH"<<std::endl;
        return {};
    }

    if(EVP_PKEY_derive_init(ctx) <= 0) {
        std::cerr<<"Couldn't initialize deriving context for DH"<<std::endl;
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    if(EVP_PKEY_derive_set_peer(ctx, peerk) <= 0) {
        std::cerr<<"Couldn't set peer for DH deriving context"<<std::endl;
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    //derives DH shared secret and returns it in 'secret'
    if(EVP_PKEY_derive(ctx, NULL, &secretlen) <= 0) {
        std::cerr<<"Couldn't derive DH key length"<<std::endl;
        EVP_PKEY_CTX_free(ctx);
        return {};
    }
    secret.resize(secretlen);

    if(EVP_PKEY_derive(ctx, secret.data(), &secretlen) <= 0) {
        std::cerr<<"Couldn't derive DH key"<<std::endl;
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    EVP_PKEY_CTX_free(ctx);

    return secret;
}

sec::sessionKey sec::keyFromSecret(std::string secret) {
    sessionKey k;
    auto result = sec::Hash(std::string(secret.begin(), secret.begin()));
    auto reskey = result.substr(0, SYMMLEN/8);
    auto resiv = result.substr(SYMMLEN/8, 16);
    memcpy(&k.key[0], reskey.data(), SYMMLEN/8);
    memcpy(&k.iv[0], resiv.data(), 16);

    return k;
}