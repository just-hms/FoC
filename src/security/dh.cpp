#include "security.h"
#include <cstdint>
#include <vector>

// Encode the public key as a string
std::tuple<std::vector<uint8_t>, entity::Error> sec::encodePeerKey(EVP_PKEY* publicKey) {

    BIO* bio = BIO_new(BIO_s_mem());
    std::vector<uint8_t>res;

    if(bio == NULL) {
        std::cerr<<"Couldn't allocate BIO structure"<<std::endl;
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }

    if(PEM_write_bio_PUBKEY(bio, publicKey) <= 0) {
        std::cerr<<"Couldn't write public key"<<std::endl;
        BIO_free(bio);
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }

    char *buffer;
    res.resize(BIO_get_mem_data(bio, &buffer));
    memcpy(res.data(), buffer, res.size());
    
    BIO_free(bio);
    return {res, entity::ERR_OK};
}

// TODO : fix memory leak
std::tuple<EVP_PKEY*, entity::Error> sec::decodePeerKey(std::vector<uint8_t> encodedKey) {
    
    BIO* bio;
    EVP_PKEY* publicKey;
    bio = BIO_new_mem_buf(encodedKey.data(), encodedKey.size());
    if(!(publicKey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL))) {
        std::cerr<<"Couldn't read public key"<<std::endl;
        BIO_free(bio);
        return {NULL, entity::ERR_BROKEN};
    }

    BIO_free(bio);
    return {publicKey, entity::ERR_OK};
}

//generate g and p
std::tuple<std::vector<uint8_t>, entity::Error> sec::genDHparam(EVP_PKEY *&params) {

    params = NULL;

    DH *tmp = DH_get_2048_256();
    std::vector<uint8_t>res;

    if(tmp == NULL) {
        std::cerr<<"Couldn't create DH"<<std::endl;
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }

    if(!(params = EVP_PKEY_new())) {
        std::cerr<<"Couldn't create DH"<<std::endl;
        DH_free(tmp);
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }

    if(EVP_PKEY_set1_DH(params, tmp) <= 0) {
        std::cerr<<"Couldn't set DH"<<std::endl;
        DH_free(tmp);
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }

    unsigned char *buffer = NULL;
    int len = i2d_DHparams(tmp, &buffer);
    if(len <= 0) {
        std::cerr<<"Couldn't encode DH"<<std::endl;
        DH_free(tmp);
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }

    res.resize(len);
    memcpy(res.data(), buffer, res.size());

    DH_free(tmp);
    
    return {res, entity::ERR_OK};
}

std::tuple<EVP_PKEY*, entity::Error> sec::retrieveDHparam(std::vector<uint8_t> DHserialized) {
    unsigned char *buffer = new unsigned char[DHserialized.size()];
    memcpy(buffer, DHserialized.data(), DHserialized.size());
    DH *tmp = d2i_DHparams(NULL, (const unsigned char**)&buffer, DHserialized.size());
    if(tmp == NULL) {
        std::cerr<<"Couldn't retrieve DH"<<std::endl;
        return {NULL, entity::ERR_BROKEN};
    }

    EVP_PKEY *params;

    if(!(params = EVP_PKEY_new())) {
        std::cerr<<"Couldn't create DH"<<std::endl;
        DH_free(tmp);
        return {NULL, entity::ERR_BROKEN};
    }

    if(EVP_PKEY_set1_DH(params, tmp) <= 0) {
        std::cerr<<"Couldn't set DH"<<std::endl;
        DH_free(tmp);
        return {NULL, entity::ERR_BROKEN};
    }

    DH_free(tmp);
    return {params, entity::ERR_OK};
}

entity::Error sec::genDH(EVP_PKEY *&pubk, EVP_PKEY *params) {

    EVP_PKEY_CTX *ctx; 

    //generate keys
    if(!(ctx = EVP_PKEY_CTX_new(params, NULL))) {
        std::cerr<<"Couldn't create a context for DH"<<std::endl;
        return entity::ERR_BROKEN;
    }

    if(EVP_PKEY_keygen_init(ctx) <= 0) {
        std::cerr<<"Couldn't initialize context for DH"<<std::endl;
        EVP_PKEY_CTX_free(ctx);
        return entity::ERR_BROKEN;
    }

    pubk = NULL;
    if(EVP_PKEY_keygen(ctx, &pubk) <= 0) {
        std::cerr<<"Couldn't generate key for DH"<<std::endl;
        EVP_PKEY_CTX_free(ctx);
        return entity::ERR_BROKEN;
    }

    EVP_PKEY_CTX_free(ctx);

    return entity::ERR_OK;
}

std::tuple<std::vector<uint8_t>, entity::Error> sec::derivateDH(EVP_PKEY *privk, EVP_PKEY *peerk) {
    EVP_PKEY_CTX *ctx;
    std::vector<uint8_t> secret;
    size_t secretlen;

    if(!(ctx = EVP_PKEY_CTX_new(privk, NULL))) {
        std::cerr<<"Couldn't create a context for DH"<<std::endl;
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }

    if(EVP_PKEY_derive_init(ctx) <= 0) {
        std::cerr<<"Couldn't initialize deriving context for DH"<<std::endl;
        EVP_PKEY_CTX_free(ctx);
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }

    if(EVP_PKEY_derive_set_peer(ctx, peerk) <= 0) {
        std::cerr<<"Couldn't set peer for DH deriving context"<<std::endl;
        EVP_PKEY_CTX_free(ctx);
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }

    //derives DH shared secret and returns it in 'secret'
    if(EVP_PKEY_derive(ctx, NULL, &secretlen) <= 0) {
        std::cerr<<"Couldn't derive DH key length"<<std::endl;
        EVP_PKEY_CTX_free(ctx);
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }
    secret.resize(secretlen);

    if(EVP_PKEY_derive(ctx, secret.data(), &secretlen) <= 0) {
        std::cerr<<"Couldn't derive DH key"<<std::endl;
        EVP_PKEY_CTX_free(ctx);
        return {std::vector<uint8_t>(), entity::ERR_BROKEN};
    }

    EVP_PKEY_CTX_free(ctx);

    return {secret, entity::ERR_OK};
}

std::tuple<std::vector<uint8_t>, entity::Error> sec::keyFromSecret(std::vector<uint8_t> secret) {

    std::vector<uint8_t> k(SYMMLEN/8);

    auto [result, err] = sec::Hash(std::string(secret.begin(), secret.end()));
    if (err != entity::ERR_OK) return {std::vector<uint8_t>(), err};

    auto reskey = result.substr(0, SYMMLEN/8);
    memcpy(k.data(), reskey.data(), SYMMLEN/8);

    return {k, entity::ERR_OK};
}