#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

#define PUBK "pubk.pem"
#define PRIVK "privk.pem"

using namespace std;

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
