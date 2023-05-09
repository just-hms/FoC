#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/pem.h>

using namespace std;

#define SYMMLEN 256
#define PUBK "pubk.pem"
#define PRIVK "privk.pem"
#define PATHserver "./server/"
#define PATHclients "./clients/"

struct message {

    uint8_t type;
    uint32_t timestamp;
    string sender, receiver;
    string buffer;

} typedef message;

struct ciphertext {
    unsigned char *body;
    size_t len = 0;

} typedef ciphertext;

class AsymCrypt {
    unordered_map<uint8_t, EVP_PKEY*> public_keys;

    public:
        AsymCrypt(string, string);
        ciphertext encrypt(unsigned char*, uint8_t);
        string decrypt(ciphertext);
};

//loads users's public key into a hash map and assigns to each user a progressive ID
AsymCrypt::AsymCrypt(string user, string pwd) {
    EVP_PKEY *key;
    FILE *fp;

    //loads every user's public key into a hash map and assigns a progressive userID to each one of them
    uint8_t userID = 0;
    for(const auto &entry: filesystem::directory_iterator(PATHclients)) {
        fp = fopen(entry.path().c_str(), "r");
        key = EVP_PKEY_new();
        key = PEM_read_PUBKEY(fp, NULL, NULL, NULL);
        fclose(fp);

        this->public_keys.insert({userID, key});
        userID++;
    }
}

//encrypts mes using userID's public key
ciphertext AsymCrypt::encrypt(unsigned char* mess, uint8_t destID) {
    EVP_PKEY_CTX *ctx;
    EVP_PKEY *key;
    ciphertext c;
    size_t inlen = strlen((char*)mess), ctlen;
    unsigned char *ct;

    key = this->public_keys[destID];

    ctx = EVP_PKEY_CTX_new(key, NULL);
    if (!ctx) {
        cout<<"a"<<endl;
        cerr<<"Unable to create a context for RSA"<<endl;
        return c;
    }
    EVP_PKEY_encrypt_init(ctx);
    EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);

    //determine output buffer length
    if(EVP_PKEY_encrypt(ctx, NULL, &ctlen, mess, inlen) <= 0) {
        cout<<"Unable to determine ct buffer lenght"<<endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);
        return c;
    }
    ct = new unsigned char[ctlen];

    if(EVP_PKEY_encrypt(ctx, ct, &ctlen, mess, inlen) == -1) {
        cerr<<"Error during encryption"<<endl;
        delete[] ct;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);
        return c;
    }

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(key);

    c.body = ct;
    c.len = ctlen;
    return c;
}

//decrypts mess using the server's private key
string AsymCrypt::decrypt(ciphertext ct) {
    EVP_PKEY_CTX *ctx;
    EVP_PKEY *key;
    size_t ptlen;
    unsigned char* pt;

    FILE *fp = fopen("./server/serverprivk.pem", "r");
    if(fp == NULL) {
        cerr<<"Couldn't open private key file"<<endl;
        return "";
    }
    key = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
    fclose(fp);

    ctx = EVP_PKEY_CTX_new(key, NULL);
    if (!ctx) {
        cerr<<"Unable to create a context for RSA"<<endl;
        return "";
    }

    EVP_PKEY_decrypt_init(ctx);
    EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);

    if(EVP_PKEY_decrypt(ctx, NULL, &ptlen, ct.body, ct.len) <= 0) {
        cerr<<"Unable to determine pt buffer length"<<endl;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);
        return "";
    }
    pt = new unsigned char[ptlen];

    if(EVP_PKEY_decrypt(ctx, pt, &ptlen, ct.body, ct.len) <= 0) {
        cerr<<"Error during decryption"<<endl;
        delete[] pt;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(key);
        return "";
    }

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(key);

    return string((char*) pt);
}

//generates a pair of public and private RSA keys, stores them in .pem files while encrypting the private key
void generateRSAkeys(string user, string pwd, unsigned int bits) {
    FILE *fp;
    EVP_PKEY *RSAkeys = EVP_RSA_gen(bits);

    fp = fopen((PATHserver+user+PUBK).c_str(), "w");
    PEM_write_PUBKEY(fp, RSAkeys);
    fclose(fp);

    fp = fopen((PATHserver+user+PRIVK).c_str(), "w");
    PEM_write_PrivateKey(fp, RSAkeys, NULL, NULL, 0, NULL, NULL);
    //EVP_aes_128_cbc(), (const unsigned char*) pwd.c_str(), pwd.size(), NULL, NULL);
    fclose(fp);

    EVP_PKEY_free(RSAkeys);
}

//stores key and iv for AES cipher
struct sessionKey {
    unsigned char key[SYMMLEN/8], iv[SYMMLEN/8];
} typedef sessionKey;

class SymCrypt {
    unordered_map<uint8_t, sessionKey> session_keys;

    public:
        void refresh(uint8_t);
        unsigned char* encrypt(uint8_t, unsigned char*);
        unsigned char* decrypt(uint8_t, unsigned char*);
};

//creates (or updates if already exists) a new key and iv to communicate with userID
void SymCrypt::refresh(uint8_t userID) {
    sessionKey k;
    RAND_bytes(k.key, sizeof(k.key));
    RAND_bytes(k.iv, sizeof(k.iv));
    this->session_keys[userID] = k;
}

//encrypts pt by using the userID's session key
unsigned char* SymCrypt::encrypt(uint8_t userID, unsigned char *pt) {
    EVP_CIPHER_CTX *ctx;
    int ptlen = strlen((char*)pt)+1, ctlen, len;
    unsigned char *ct;
    sessionKey k = this->session_keys[userID];

    if(!(ctx = EVP_CIPHER_CTX_new())) {
        cerr<<"Unable to create a context for AES"<<endl;
        return NULL;
    }

    if(EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, k.key, k.iv) <= 0) {
        cerr<<"Unable to create a initialize context for AES"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    ct = new unsigned char[ptlen+16];

    if(EVP_EncryptUpdate(ctx, ct, &len, pt, ptlen) <= 0) {
        cerr<<"Unable to encrypt with AES"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        delete[] ct;
        return NULL;
    }
    ctlen = len;

    if(EVP_EncryptFinal_ex(ctx, ct + len, &len) <= 0)
    ctlen += len;

    EVP_CIPHER_CTX_free(ctx);
    return ct;
    
}

//decrypts ct by using the userID's session key
unsigned char* SymCrypt::decrypt(uint8_t userID, unsigned char *ct) {
    EVP_CIPHER_CTX *ctx;
    int ctlen = strlen((char*)ct)+1, ptlen, len;
    unsigned char *pt;
    sessionKey k = this->session_keys[userID];

    if(!(ctx = EVP_CIPHER_CTX_new())) {
        cerr<<"Unable to create a context for AES"<<endl;
        return NULL;
    }

    if(EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, k.key, k.iv) <= 0) {
        cerr<<"Unable to create a initialize context for AES"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    pt = new unsigned char[ctlen];

    if(EVP_DecryptUpdate(ctx, pt, &len, ct, ctlen) <= 0) {
        cerr<<"Unable to decrypt with AES"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        delete[] pt;
        return NULL;
    }
    ptlen = len;

    if(EVP_DecryptFinal_ex(ctx, pt + len, &len) <= 0)
    ptlen += len;

    EVP_CIPHER_CTX_free(ctx);
    return pt;
}

int main() {
    //generateRSAkeys("server", "culo", 4096);
    //AsymCrypt a("server", "culo");
    //ciphertext c = a.encrypt((unsigned char*)"diobastardo\n", 0);
    //if(c.len != 0) {string s = a.decrypt(c); cout<<s<<endl;}

    SymCrypt b;
    b.refresh(0);
    unsigned char *enc = b.encrypt(0, (unsigned char*)"mannaggia al cristo e tutti gli angeli in colonna\n");
    cout<<string((char*)b.decrypt(0, enc))<<endl;
    return 0;
}