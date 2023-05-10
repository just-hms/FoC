#include "password.h"

//takes in input a password and a salt, produces the hash using SHA3-512
string HashAndSalt(string pwd, string salt) {

    unsigned int hashlen;
    char *hash = new char[EVP_MD_size(EVP_sha3_512())];

    //concatenation
    string m = pwd + salt;

    //hashing
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit(ctx, EVP_sha3_512());
    EVP_DigestUpdate(ctx, (unsigned char*)m.c_str(), m.size());
    EVP_DigestFinal(ctx, (unsigned char*)hash, &hashlen);
    EVP_MD_CTX_free(ctx);

    return string(hash);
}

//takes in input the hash of a pwd, pwd in clear and the salt, returns 1 if the hash corresponds
bool VerifyHash(string hashedPWD, string pwd, string salt) {
    string computedHash = HashAndSalt(pwd, salt);
    if(CRYPTO_memcmp((void*) &computedHash[0], (void*) &hashedPWD[0], EVP_MD_size(EVP_sha3_512())) == 0) return true;
    return false;
}
