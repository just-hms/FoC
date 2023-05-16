#include "test.h"
#include <cstdint>
#include <vector>

std::vector<uint8_t> mess = {'m', 'e', 's', 's', 'a', 'g', 'e'};

int TestDH(){
    EVP_PKEY *p, *sdh, *cdh;

    auto error = genDHparam(p);
    ASSERT_FALSE(error < 0);
    
    error = genDH(sdh, p);
    ASSERT_FALSE(error < 0);

    error = genDH(cdh, p);
    ASSERT_FALSE(error < 0);

    auto lvector = derivateDH(sdh, cdh);
    auto rvector = derivateDH(cdh, sdh);

    ASSERT_TRUE(lvector == rvector);

    auto key = keyFromSecret(std::string(lvector.begin(), lvector.end()));

    SymCrypt R(key);

    auto res = R.decrypt(
        R.encrypt( mess)       
    );

    ASSERT_TRUE(mess == res);

    EVP_PKEY_free(p);
    EVP_PKEY_free(sdh);
    EVP_PKEY_free(cdh);

    TEST_PASSED();
}

int TestRSA(){
    
    // RSA GENERATION
    auto err = generateRSAkeys("server", "server", 4096);
    ASSERT_FALSE(err < 0);

    err = generateRSAkeys("client", "client", 4096);
    ASSERT_FALSE(err < 0);

    // AsymCrypt
    AsymCrypt AS("serverprivk.pem", "clientpubk.pem", "server");
    AsymCrypt AC("clientprivk.pem", "serverpubk.pem", "client");


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

    sessionKey symk;
    RAND_bytes(symk.key, SYMMLEN/8);
    RAND_bytes(symk.iv, 16);

    SymCrypt SC(symk);
    auto res = SC.decrypt(
        SC.encrypt(mess)
    );
    ASSERT_TRUE(mess == res);

    RAND_bytes(symk.key, SYMMLEN/8);
    RAND_bytes(symk.iv, SYMMLEN/8);
    SC.refresh(symk);

    res = SC.decrypt(
        SC.encrypt(mess)
    );
    ASSERT_TRUE(mess == res);

    TEST_PASSED();
}

int TestHash(){
    std::string mess = "message";
    ASSERT_TRUE(Hash(mess) == Hash(mess));

    TEST_PASSED();
}

int TestHashAndSalt(){
    std::string password = "secret";

    auto hashed = HashAndSalt(password);

    ASSERT_TRUE(VerifyHash(hashed, password));

    TEST_PASSED();
}

int TestMAC() {
    std::string mess = "message";

    Hmac h("");
    ASSERT_TRUE(h.MAC(mess) == h.MAC(mess));

    TEST_PASSED();
}
