#include "test.h"
#include <cstdint>
#include <vector>

std::vector<uint8_t> mess = {'m', 'e', 's', 's', 'a', 'g', 'e'};

#define DATA_PATH std::string("./data/")

int TestDH(){
    //parameter generation, key generation and derivation
    EVP_PKEY *p = NULL, *sdh = NULL, *cdh = NULL;

    auto DHserialized = sec::genDHparam(p);
    ASSERT_FALSE(DHserialized.size() == 0);
    
    auto error = sec::genDH(sdh, p);
    ASSERT_FALSE(error < 0);

    error = sec::genDH(cdh, p);
    ASSERT_FALSE(error < 0);

    auto lvector = sec::derivateDH(sdh, cdh);
    auto rvector = sec::derivateDH(cdh, sdh);

    ASSERT_TRUE(lvector == rvector);

    //DH parameters communication
    EVP_PKEY *p2 = NULL;
    DHserialized = sec::genDHparam(p2);
    ASSERT_FALSE(sec::genDHparam(p) != sec::genDHparam(p2));
    EVP_PKEY *p3 = sec::retrieveDHparam(DHserialized); 
    ASSERT_TRUE(DHserialized == sec::genDHparam(p3));

    //check if using different parameters leads to different shared secret
    sec::genDHparam(p2);
    EVP_PKEY *sdh2 = NULL;
    sec::genDH(sdh2, p2);
    ASSERT_TRUE(sec::derivateDH(cdh, sdh2) != sec::derivateDH(cdh, sdh));

    //check if both parties derive the same key
    auto k1 = sec::keyFromSecret(lvector);
    auto k2 = sec::keyFromSecret(rvector);
    std::vector<uint8_t> tmp1(32), tmp2(16), tmp3(32), tmp4(16);

    //  :-)

    memcpy(tmp1.data(), k1.key, 32);
     memcpy(tmp2.data(), k1.iv, 16);
      memcpy(tmp3.data(), k2.key, 32);
       memcpy(tmp4.data(), k2.iv, 16);

        // :-)

    ASSERT_TRUE((tmp1 == tmp3) && (tmp2 == tmp4));

    sec::SymCrypt R(k1);

    auto res = R.decrypt(
        R.encrypt(mess)       
    );

    ASSERT_TRUE(mess == res);

    EVP_PKEY_free(p);
    EVP_PKEY_free(sdh);
    EVP_PKEY_free(cdh);

    TEST_PASSED();
}

int TestRSA(){
    
    // RSA GENERATION
    auto err = sec::generateRSAkeys(DATA_PATH + "server", "sercret_server", 4096);
    ASSERT_FALSE(err < 0);

    err = sec::generateRSAkeys(DATA_PATH + "client", "sercret_client", 4096);
    ASSERT_FALSE(err < 0);

    // AsymCrypt
    sec::AsymCrypt AS(DATA_PATH + "serverprivk.pem", DATA_PATH + "clientpubk.pem", "sercret_server");
    sec::AsymCrypt AC(DATA_PATH + "clientprivk.pem", DATA_PATH + "serverpubk.pem", "sercret_client");

    // one way
    auto res = AC.decrypt(
        AS.encrypt(mess)
    );
    ASSERT_TRUE(mess == res);

    // the other  way
    res = AS.decrypt(
        AC.encrypt(mess)
    );
    ASSERT_TRUE(mess == res);

    TEST_PASSED();
}

int TestAES(){

    sec::sessionKey symk;
    RAND_bytes(symk.key, SYMMLEN/8);
    RAND_bytes(symk.iv, 16);

    sec::SymCrypt SC(symk);
    auto res = SC.decrypt(
        SC.encrypt(mess)
    );
    ASSERT_TRUE(mess == res);

    res = SC.decrypt(
        SC.encrypt(mess)
    );
    ASSERT_TRUE(mess == res);

    EVP_PKEY *dh, *pubs;
    auto a = sec::genDHparam(dh);
    sec::genDH(pubs, dh);
    auto aaa = sec::encodePublicKey(pubs);
    SC.encrypt(aaa);

    TEST_PASSED();
}

int TestHash(){
    std::string mess = "message";
    ASSERT_TRUE(sec::Hash(mess) == sec::Hash(mess));

    TEST_PASSED();
}

int TestHashAndSalt(){
    std::string password = "secret";

    auto hashandsalt = sec::HashAndSalt(password);

    ASSERT_TRUE(sec::VerifyHash(hashandsalt, password));

    TEST_PASSED();
}

int TestMAC() {

    sec::Hmac h;
    ASSERT_TRUE(h.MAC(mess) == h.MAC(mess));

    TEST_PASSED();
}

int TestEncodeEVP_PKEY() {
    EVP_PKEY *key = EVP_RSA_gen(1024);

    ASSERT_TRUE(sec::encodePublicKey(sec::decodePublicKey(sec::encodePublicKey(key))) == sec::encodePublicKey(key));

    TEST_PASSED();
}

int main() {
    return TestAES();
}