#include <iostream>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <openssl/evp.h>
#include <openssl/pem.h>

using namespace std;

//copies udata into buf and returns the amount of bytes copied
int pem_password_callback(char *buf, int max_len, int flag, void *udata) {
    char* pwd = (char*) udata;
    int len = strlen(pwd);

    if(len > max_len)
        return 0;

    memcpy(buf, pwd, len);
    return len;
}

EVP_PKEY* private_key_cb() {

    FILE *fp = fopen(<path della chiave privata>, "r");
    if(fp == NULL) {
        cerr<<"Couldn't open private key file"<<endl;
        return NULL;
    }

    EVP_PKEY *key = PEM_read_PrivateKey(fp, NULL, pem_password_callback, <pwd usata per cifrare la chiave privata>);
    fclose(fp);
    
    return key;
}

unordered_map<uint8_t, EVP_PKEY*> load_public_keys(string pubk_path) {
    unordered_map<uint8_t, EVP_PKEY*> public_keys;
    EVP_PKEY *key;
    FILE *fp;

    //loads every user's public key into a hash map and assigns a progressive userID to each one of them
    uint8_t userID = 0;
    for(const auto &entry: filesystem::directory_iterator(pubk_path)) {
        fp = fopen(entry.path().c_str(), "r");
        key = PEM_read_PUBKEY(fp, NULL, NULL, NULL);
        fclose(fp);

        public_keys.insert({userID, key});
        userID++;
    }

    return public_keys;
}