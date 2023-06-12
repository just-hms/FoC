#include "security.h"
#include <cstdint>
#include <vector>

namespace sec {

    // Encode the public key as a string
    std::tuple<std::vector<uint8_t>, entity::Error> encodePeerKey(EVP_PKEY* publicKey) {

        BIO* bio = BIO_new(BIO_s_mem());
        if(bio == NULL) {
            std::cerr<<"Couldn't allocate BIO structure"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        defer { BIO_free(bio); };

        if(PEM_write_bio_PUBKEY(bio, publicKey) <= 0) {
            std::cerr<<"Couldn't write public key"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        char *buffer;
        std::vector<uint8_t>res;

        res.resize(BIO_get_mem_data(bio, &buffer));
        memcpy(res.data(), buffer, res.size());
        
        return {res, entity::ERR_OK};
    }

    std::tuple<EVP_PKEY*, entity::Error> decodePeerKey(std::vector<uint8_t> encodedKey) {
        BIO* bio = BIO_new_mem_buf(encodedKey.data(), encodedKey.size());
        defer { BIO_free(bio); };

        EVP_PKEY* publicKey;
        if(!(publicKey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL))) {
            std::cerr<<"Couldn't read public key"<<std::endl;
            return {NULL, entity::ERR_BROKEN};
        }

        return {publicKey, entity::ERR_OK};
    }

    //generate g and p
    std::tuple<std::vector<uint8_t>, entity::Error> genDHparam(EVP_PKEY *&params) {
        DH *tmp = DH_get_2048_256();
        if(tmp == NULL) {
            std::cerr<<"Couldn't create DH"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }
        defer { DH_free(tmp); };

        params = NULL;
        if(!(params = EVP_PKEY_new())) {
            std::cerr<<"Couldn't create DH"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        if(EVP_PKEY_set1_DH(params, tmp) <= 0) {
            std::cerr<<"Couldn't set DH"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        unsigned char *buffer = NULL;
        int len = i2d_DHparams(tmp, &buffer);
        if(len <= 0) {
            std::cerr<<"Couldn't encode DH"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        std::vector<uint8_t>res(len);
        memcpy(res.data(), buffer, res.size());
        
        return {res, entity::ERR_OK};
    }

    std::tuple<EVP_PKEY*, entity::Error> retrieveDHparam(std::vector<uint8_t> DHserialized) {
        unsigned char *buffer = new unsigned char[DHserialized.size()];
        memcpy(buffer, DHserialized.data(), DHserialized.size());
        DH *tmp = d2i_DHparams(NULL, (const unsigned char**)&buffer, DHserialized.size());
        if(tmp == NULL) {
            std::cerr<<"Couldn't retrieve DH"<<std::endl;
            return {NULL, entity::ERR_BROKEN};
        }
        defer { DH_free(tmp); }; 
        
        EVP_PKEY *params;
        if(!(params = EVP_PKEY_new())) {
            std::cerr<<"Couldn't create DH"<<std::endl;
            return {NULL, entity::ERR_BROKEN};
        }

        if(EVP_PKEY_set1_DH(params, tmp) <= 0) {
            std::cerr<<"Couldn't set DH"<<std::endl;
            return {NULL, entity::ERR_BROKEN};
        }

        return {params, entity::ERR_OK};
    }

    entity::Error genDH(EVP_PKEY *&pubk, EVP_PKEY *params) {

        EVP_PKEY_CTX *ctx; 

        //generate keys
        if(!(ctx = EVP_PKEY_CTX_new(params, NULL))) {
            std::cerr<<"Couldn't create a context for DH"<<std::endl;
            return entity::ERR_BROKEN;
        }
        defer {EVP_PKEY_CTX_free(ctx); };

        if(EVP_PKEY_keygen_init(ctx) <= 0) {
            std::cerr<<"Couldn't initialize context for DH"<<std::endl;
            return entity::ERR_BROKEN;
        }

        pubk = NULL;
        if(EVP_PKEY_keygen(ctx, &pubk) <= 0) {
            std::cerr<<"Couldn't generate key for DH"<<std::endl;
            return entity::ERR_BROKEN;
        }
        return entity::ERR_OK;
    }

    std::tuple<std::vector<uint8_t>, entity::Error> derivateDH(EVP_PKEY *privk, EVP_PKEY *peerk) {
        EVP_PKEY_CTX *ctx;

        if(!(ctx = EVP_PKEY_CTX_new(privk, NULL))) {
            std::cerr<<"Couldn't create a context for DH"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }
        defer {EVP_PKEY_CTX_free(ctx);};

        if(EVP_PKEY_derive_init(ctx) <= 0) {
            std::cerr<<"Couldn't initialize deriving context for DH"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        if(EVP_PKEY_derive_set_peer(ctx, peerk) <= 0) {
            std::cerr<<"Couldn't set peer for DH deriving context"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        //derives DH shared secret and returns it in 'secret'
        size_t secretlen;

        if(EVP_PKEY_derive(ctx, NULL, &secretlen) <= 0) {
            std::cerr<<"Couldn't derive DH key length"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        std::vector<uint8_t> secret(secretlen);
        if(EVP_PKEY_derive(ctx, secret.data(), &secretlen) <= 0) {
            std::cerr<<"Couldn't derive DH key"<<std::endl;
            return {std::vector<uint8_t>(), entity::ERR_BROKEN};
        }

        return {secret, entity::ERR_OK};
    }

    std::tuple<std::vector<uint8_t>, entity::Error> keyFromSecret(std::vector<uint8_t> secret) {

        std::vector<uint8_t> k(SYMMLEN/8+16);

        auto [result, err] = Hash(std::string(secret.begin(), secret.end()));
        if (err != entity::ERR_OK) return {std::vector<uint8_t>(), err};

        auto seskey = result.substr(0, SYMMLEN/8);
        auto mackey = result.substr(result.size()-16, 16);
        memcpy(k.data(), seskey.data(), SYMMLEN/8);
        memcpy(k.data()+SYMMLEN/8, mackey.data(), 16);

        return {k, entity::ERR_OK};
    }
}