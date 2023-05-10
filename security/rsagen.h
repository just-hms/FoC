#include <iostream>
#include <string>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

using namespace std;

#define PUBK "pubk.pem"
#define PRIVK "privk.pem"

void generateRSAkeys(string, string, unsigned int);