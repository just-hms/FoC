#include "security.h"

namespace sec {

// generates a pair of public and private RSA keys, stores them in .pem while
// encrypting the private key using pwd path specifies both the folder and the
// user -> "./folder/username" bits specifies how many bits to use for the RSA
// keys
int generateRSAkeys(std::string path, std::string pwd, unsigned int bits) {
    // generate RSA key
    EVP_PKEY *RSAkeys = EVP_RSA_gen(bits);
    defer { EVP_PKEY_free(RSAkeys); };

    // write public key
    FILE *fpPub = fopen((path + PUBK).c_str(), "w");
    if (fpPub == NULL) {
        std::cerr << "Unable to create public key file" << std::endl;
        return entity::ERR_BROKEN;
    }
    defer { fclose(fpPub); };

    if (PEM_write_PUBKEY(fpPub, RSAkeys) <= 0) {
        std::cerr << "Couldn't save public key" << std::endl;
        return entity::ERR_BROKEN;
    }

    // write private key
    FILE *fpPriv = fopen((path + PRIVK).c_str(), "w");
    if (fpPriv == NULL) {
        std::cerr << "Unable to create private key file" << std::endl;
        return entity::ERR_BROKEN;
    }
    defer { fclose(fpPriv); };

    if (PEM_write_PrivateKey(fpPriv, RSAkeys, EVP_aes_128_cbc(),
                             (const unsigned char *)pwd.c_str(), pwd.size(),
                             NULL, NULL) <= 0) {
        std::cerr << "Couldn't save private key" << std::endl;
        return entity::ERR_BROKEN;
    }
    return entity::ERR_OK;
}
}  // namespace sec