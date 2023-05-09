#include <string>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

using namespace std;

#define PUBK "pubk.pem"
#define PRIVK "privk.pem"

//generates a pair of public and private RSA keys, stores them in .pem files while encrypting the private key
void generateRSAkeys(string path, string pwd, unsigned int bits) {
    FILE *fp;
    EVP_PKEY *RSAkeys = EVP_RSA_gen(bits);

    fp = fopen((path+PUBK).c_str(), "w");
    PEM_write_PUBKEY(fp, RSAkeys);
    fclose(fp);

    fp = fopen((path+PRIVK).c_str(), "w");
    PEM_write_PrivateKey(fp, RSAkeys, NULL, NULL, 0, NULL, NULL);
    //EVP_aes_128_cbc(), (const unsigned char*) pwd.c_str(), pwd.size(), NULL, NULL);
    fclose(fp);

    EVP_PKEY_free(RSAkeys);
}