#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/pem.h>

using namespace std;

#define SYMMLEN 256
#define PUBK "pubk.pem"
#define PRIVK "privk.pem"

using namespace std;

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
    function<EVP_PKEY* ()> privk_cb;
    unordered_map<uint8_t, EVP_PKEY*> public_keys;

    public:
        AsymCrypt(function<EVP_PKEY* ()>, unordered_map<uint8_t, EVP_PKEY*>);
        ciphertext encrypt(unsigned char*, uint8_t);
        string decrypt(ciphertext);
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
    unordered_map<uint8_t, sessionKey> session_keys;

    public:
        SymCrypt(uint8_t);
        void refresh(uint8_t);
        unsigned char* encrypt(uint8_t, unsigned char*);
        unsigned char* decrypt(uint8_t, unsigned char*);
        ~SymCrypt();
};

// RSAGEN
void generateRSAkeys(string, string, unsigned int);


// PASSWORD
string HashAndSalt(string, string);
bool VerifyHash(string, string, string);