#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/dh.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/pem.h>

#define SYMMLEN 256
#define SALT_LEN 16
#define HMAC_KEY_LEN 16
#define PUBK "pubk.pem"
#define PRIVK "privk.pem"

// ASYMCRYPT
struct ciphertext {
    unsigned char *body;
    size_t len = 0;

} typedef ciphertext;


/*EXAMPLE
AsymCrypt a(private_key_cb, load_public_keys("./clients/"));
ciphertext c = a.encrypt((unsigned char*)"Lorem ipsum dolor sit amet\n", 0);
if(c.len != 0) {
    string s = a.decrypt(c);
    cout<<s<<endl;
}
*/
class AsymCrypt {
    std::string privk;
    std::string pubk;
    std::string privk_pwd;

    public:
        AsymCrypt(std::string, std::string, std::string);
        ciphertext encrypt(unsigned char*);
        std::string decrypt(ciphertext);
        ~AsymCrypt();
};


// SYMCRYPT
//stores key and iv for AES cipher
struct sessionKey {
    unsigned char key[SYMMLEN/8], iv[SYMMLEN/8];
} typedef sessionKey;


/*EXAMPLE
SymCrypt b(0);
unsigned char *enc = b.encrypt(0, (unsigned char*) "Lorem ipsum dolor sit amet\n");
cout<<string((char*) b.decrypt(0, enc))<<endl;
*/
class SymCrypt {
    sessionKey key;

    public:
        SymCrypt(sessionKey);
        void refresh(sessionKey);
        unsigned char* encrypt(unsigned char*);
        unsigned char* decrypt(unsigned char*);
        ~SymCrypt();
};

// RSAGEN
void generateRSAkeys(std::string, std::string, unsigned int);

class Hmac {
    unsigned char key[16];

    public:
        Hmac();
        Hmac(std::string);
        std::string MAC(std::string);
};

// PASSWORD

std::string Hash(std::string);
std::string HashAndSalt(std::string, std::string);
bool VerifyHash(std::string, std::string);

//ENCODING

std::string encode(char*, int);