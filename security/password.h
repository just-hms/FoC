#include <string>
#include <openssl/evp.h>
using namespace std;

string HashAndSalt(string, string);
bool VerifyHash(string, string, string);