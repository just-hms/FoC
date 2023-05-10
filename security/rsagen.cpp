#include "rsagen.h"

//generates a pair of public and private RSA keys, stores them in .pem while encrypting the private key using pwd
//path specifies both the folder and the user -> "./folder/username"
//bits specifies how many bits to use for the RSA keys
void generateRSAkeys(string path, string pwd, unsigned int bits) {
    FILE *fp;
    EVP_PKEY *RSAkeys = EVP_RSA_gen(bits);

    fp = fopen((path+PUBK).c_str(), "w");
    if(fp == NULL) {
        cerr<<"Unable to create public key file"<<endl;
        return;
    }
    PEM_write_PUBKEY(fp, RSAkeys);
    fclose(fp);

    fp = fopen((path+PRIVK).c_str(), "w");
    if(fp == NULL) {
        cerr<<"Unable to create private key file"<<endl;
        return;
    }
    PEM_write_PrivateKey(fp, RSAkeys, EVP_aes_128_cbc(), (const unsigned char*) pwd.c_str(), pwd.size(), NULL, NULL);
    fclose(fp);

    EVP_PKEY_free(RSAkeys);
}