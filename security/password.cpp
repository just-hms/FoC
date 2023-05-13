#include "security.h"
using namespace std;

int hash_len = EVP_MD_size(EVP_sha3_512());

string encode(char *s, int len) {
    ostringstream oss;
    for (unsigned int i = 0; i < len; i++) {
        oss<<hex<<setw(2)<<setfill('0')<<static_cast<int>(s[i]);
    }
    return oss.str();
}

int decode(const string &s) {
    return strtoul(s.c_str(), NULL, 16);
}

string Hash(string m) {
    unsigned int buflen;
    char *buf = new char[hash_len];

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit(ctx, EVP_sha3_512());
    EVP_DigestUpdate(ctx, (unsigned char*)m.c_str(), m.size());
    EVP_DigestFinal(ctx, (unsigned char*)buf, &buflen);
    EVP_MD_CTX_free(ctx);

    return encode(buf, hash_len);
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
