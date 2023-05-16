#include "security.h"
using namespace std;

int hash_len = EVP_MD_size(EVP_sha3_512());

//given a buffer and its length, returns a hex string of the contents of such buffer
string encode(char *s, int len) {
    ostringstream oss;
    for (unsigned int i = 0; i < len; i++) {
        oss<<hex<<setw(2)<<setfill('0')<<static_cast<int>(s[i]);
    }
    return oss.str();
}

//creates the key from scatch
Hmac::Hmac() {
    RAND_bytes(this->key, sizeof(this->key));
}

//initialize a Hmac object with specified key, if the key is too short then it's increased in size
Hmac::Hmac(string key) {
    unsigned char *c;
    string tmp = key;

    if(key.size() < HMAC_KEY_LEN) {
        int rem = HMAC_KEY_LEN - key.size(), i = 0;
        c = new unsigned char[rem];
        RAND_bytes(c, rem);
        tmp.append(string((char*)c));
    }

    memcpy(this->key, (unsigned char*) tmp.c_str(), HMAC_KEY_LEN);
}

//builds and returns a MAC in hex string form
string Hmac::MAC(string data) {
    unsigned char *buffer = new unsigned char[EVP_MD_size(EVP_sha3_512())];
    unsigned int len;
    HMAC_CTX *ctx;
    
    if(!(ctx = HMAC_CTX_new())) {
        cerr<<"Unable to create context for HMAC"<<endl;
        return "";
    }
    if(HMAC_Init(ctx, this->key, sizeof(this->key), EVP_sha3_512()) <= 0) {
        cerr<<"Unable to initialize context for HMAC"<<endl;
        HMAC_CTX_free(ctx);
        return "";
    }
    if(HMAC_Update(ctx, (unsigned char*) data.c_str(), data.size()) <= 0) {
        cerr<<"Unable to compute HMAC"<<endl;
        HMAC_CTX_free(ctx);
        return "";
    }
    if(HMAC_Final(ctx, buffer, &len) <= 0) {
        cerr<<"Unable to compute HMAC"<<endl;
        HMAC_CTX_free(ctx);
        return "";
    }
    HMAC_CTX_free(ctx);

    string res = encode((char*) buffer, len);
    delete[] buffer;

    return res;
}

//hashes data using EVP_sha3_512, returns a hex string
string Hash(string data) {
    unsigned int buflen;
    char *buf = new char[hash_len];
    EVP_MD_CTX *ctx;

    if(!(ctx = EVP_MD_CTX_new())) {
        cerr<<"Unable to create context for Hash"<<endl;
        return "";
    }
    if(EVP_DigestInit(ctx, EVP_sha3_512()) <= 0) {
        cerr<<"Unable to initialize context for Hash"<<endl;
        EVP_MD_CTX_free(ctx);
        return "";
    }
    if(EVP_DigestUpdate(ctx, (unsigned char*)data.c_str(), data.size()) <= 0) {
        cerr<<"Unable to compute Hash"<<endl;
        EVP_MD_CTX_free(ctx);
        return "";
    }
    if(EVP_DigestFinal(ctx, (unsigned char*)buf, &buflen) <= 0) {
        cerr<<"Unable to compute Hash"<<endl;
        EVP_MD_CTX_free(ctx);
        return "";
    }
    EVP_MD_CTX_free(ctx);

    string res = encode(buf, hash_len);
    delete[] buf;

    return res;
}

//takes in input a password and salt
//if the salt is empty it generates one
//pwd and salt are concatenated and used as input to generate the hash
//returns hex(hash|salt)
string HashAndSalt(string pwd, string salt) {

    if(salt.size() == 0) {
        salt.resize(SALT_LEN);
        RAND_bytes((unsigned char*) salt.c_str(), SALT_LEN);
        salt = encode(&salt[0], SALT_LEN);
    }

    return Hash(pwd + salt)+"|"+salt;
}

//takes in input hex(hash|salt) and the pwd in the clear
//returns 1 if the computed hash corresponds to the input
bool VerifyHash(string hashedPWD, string pwd) {
    string salt = hashedPWD.substr(hashedPWD.find("|")+1, hashedPWD.size());
    string computedHash = HashAndSalt(pwd, salt);
    return (CRYPTO_memcmp((void*) &computedHash[0], (void*) &hashedPWD[0], EVP_MD_size(EVP_sha3_512())) == 0);
}
