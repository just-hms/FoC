#include <iostream>
#include <string>
#include <unordered_map>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/pem.h>

using namespace std;

#define SYMMLEN 256

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