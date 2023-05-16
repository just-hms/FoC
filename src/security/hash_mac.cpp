#include "security.h"

int hash_len = EVP_MD_size(EVP_sha3_512());

//given a buffer and its length, returns a hex string of the contents of such buffer
std::string sec::encode(char *s, int len) {
    std::ostringstream oss;
    for (unsigned int i = 0; i < len; i++) {
        oss<<std::hex<<std::setw(2)<<std::setfill('0')<<static_cast<int>(s[i]);
    }
    return oss.str();
}

//creates the key from scatch
sec::Hmac::Hmac() {
    RAND_bytes(this->key, sizeof(this->key));
}

//initialize a Hmac object with specified key, if the key is too short then it's increased in size
sec::Hmac::Hmac(std::string key) {
    unsigned char *c;
    std::string tmp = key;

    if(key.size() < HMAC_KEY_LEN) {
        int rem = HMAC_KEY_LEN - key.size(), i = 0;
        c = new unsigned char[rem];
        RAND_bytes(c, rem);
        tmp.append(std::string((char*)c));
    }

    memcpy(this->key, (unsigned char*) tmp.c_str(), HMAC_KEY_LEN);
}

//builds and returns a MAC in hex string form
std::string sec::Hmac::MAC(std::string data) {
    unsigned char *buffer = new unsigned char[EVP_MD_size(EVP_sha3_512())];
    unsigned int len;
    HMAC_CTX *ctx;
    
    if(!(ctx = HMAC_CTX_new())) {
        std::cerr<<"Unable to create context for HMAC"<<std::endl;
        return "";
    }
    if(HMAC_Init(ctx, this->key, sizeof(this->key), EVP_sha3_512()) <= 0) {
        std::cerr<<"Unable to initialize context for HMAC"<<std::endl;
        HMAC_CTX_free(ctx);
        return "";
    }
    if(HMAC_Update(ctx, (unsigned char*) data.c_str(), data.size()) <= 0) {
        std::cerr<<"Unable to compute HMAC"<<std::endl;
        HMAC_CTX_free(ctx);
        return "";
    }
    if(HMAC_Final(ctx, buffer, &len) <= 0) {
        std::cerr<<"Unable to compute HMAC"<<std::endl;
        HMAC_CTX_free(ctx);
        return "";
    }
    HMAC_CTX_free(ctx);

    std::string res = sec::encode((char*) buffer, len);
    delete[] buffer;

    return res;
}

//hashes data using EVP_sha3_512, returns a hex string
std::string sec::Hash(std::string data) {
    unsigned int buflen;
    char *buf = new char[hash_len];
    EVP_MD_CTX *ctx;

    if(!(ctx = EVP_MD_CTX_new())) {
        std::cerr<<"Unable to create context for Hash"<<std::endl;
        return "";
    }
    if(EVP_DigestInit(ctx, EVP_sha3_512()) <= 0) {
        std::cerr<<"Unable to initialize context for Hash"<<std::endl;
        EVP_MD_CTX_free(ctx);
        return "";
    }
    if(EVP_DigestUpdate(ctx, (unsigned char*)data.c_str(), data.size()) <= 0) {
        std::cerr<<"Unable to compute Hash"<<std::endl;
        EVP_MD_CTX_free(ctx);
        return "";
    }
    if(EVP_DigestFinal(ctx, (unsigned char*)buf, &buflen) <= 0) {
        std::cerr<<"Unable to compute Hash"<<std::endl;
        EVP_MD_CTX_free(ctx);
        return "";
    }
    EVP_MD_CTX_free(ctx);

    auto res = sec::encode(buf, hash_len);
    delete[] buf;

    return res;
}

//takes in input a password and salt
//if the salt is empty it generates one
//pwd and salt are concatenated and used as input to generate the hash
//returns hex(hash|salt)
std::string sec::HashAndSalt(std::string password, std::string salt) {

    if(salt.size() == 0) {
        salt.resize(SALT_LEN);
        RAND_bytes((unsigned char*) salt.c_str(), SALT_LEN);
        salt = sec::encode(&salt[0], SALT_LEN);
    }

    return sec::Hash(password + salt)+"|"+salt;
}

//takes in input hex(hash|salt) and the pwd in the clear
//returns 1 if the computed hash corresponds to the input
bool sec::VerifyHash(std::string hashandsalt, std::string password) {
    auto salt = hashandsalt.substr(hashandsalt.find("|")+1, hashandsalt.size());
    auto computedHash = sec::HashAndSalt(password, salt);
    
    return (
        CRYPTO_memcmp((void*) &computedHash[0], 
        (void*) &hashandsalt[0], 
        EVP_MD_size(EVP_sha3_512())) == 0
    );
}
