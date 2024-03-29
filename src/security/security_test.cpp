
#include "security.h"

#include <cstdint>
#include <vector>

#include "../test/test.h"
#include "./../entity/entity.h"

std::vector<uint8_t> expectedMess = {'m', 'e', 's', 's', 'a', 'g', 'e'};

int TestDH() {
    // parameter generation, key generation and derivation
    EVP_PKEY *p = NULL, *sdh = NULL, *cdh = NULL;

    auto [DHserialized, err] = sec::genDHparam(p);
    ASSERT_TRUE(err == entity::ERR_OK);

    ASSERT_FALSE(DHserialized.size() == 0);

    auto error = sec::genDH(sdh, p);
    ASSERT_FALSE(error < 0);

    error = sec::genDH(cdh, p);
    ASSERT_FALSE(error < 0);

    auto [lvector, lerr] = sec::derivateDH(sdh, cdh);
    ASSERT_TRUE(lerr == entity::ERR_OK);
    auto [rvector, rerr] = sec::derivateDH(cdh, sdh);
    ASSERT_TRUE(rerr == entity::ERR_OK);

    ASSERT_TRUE(lvector == rvector);

    // check if using different parameters leads to different shared secret
    EVP_PKEY *p2 = NULL;
    sec::genDHparam(p2);
    EVP_PKEY *sdh2 = NULL;
    sec::genDH(sdh2, p2);
    ASSERT_TRUE(sec::derivateDH(cdh, sdh2) != sec::derivateDH(cdh, sdh));

    // check if both parties derive the same key
    auto [k1, k1err] = sec::keyFromSecret(lvector);
    auto [k2, k2err] = sec::keyFromSecret(rvector);

    //  :-)

    std::vector<uint8_t> tmp1(32), tmp2(16), tmp3(32), tmp4(16);
    memcpy(tmp1.data(), k1.data(), 32);
    memcpy(tmp3.data(), k2.data(), 32);

    ASSERT_TRUE((tmp1 == tmp3) && (tmp2 == tmp4));

    // :-)

    EVP_PKEY_free(p);
    EVP_PKEY_free(sdh);
    EVP_PKEY_free(cdh);

    return 0;
}

int TestRSA() {
    auto DATA_PATH = std::string("/tmp/test-rsa/");
    std::system(("mkdir -p " + DATA_PATH).c_str());
    defer { std::system(("rm -rf " + DATA_PATH).c_str()); };

    // RSA GENERATION
    auto err = sec::generateRSAkeys(DATA_PATH + "server", "secret", 4096);
    ASSERT_FALSE(err < 0);

    err = sec::generateRSAkeys(DATA_PATH + "client", "secret", 4096);
    ASSERT_FALSE(err < 0);

    // AsymCrypt
    sec::AsymCrypt AS(DATA_PATH + "server" + sec::PRIVK,
                      DATA_PATH + "client" + sec::PUBK, "secret");
    sec::AsymCrypt AC(DATA_PATH + "client" + sec::PRIVK,
                      DATA_PATH + "server" + sec::PUBK, "secret");

    std::vector<uint8_t> enc;
    std::tie(enc, err) = AS.sign(expectedMess);
    ASSERT_TRUE(err == entity::ERR_OK);
    bool res;
    std::tie(res, err) = AC.verify(expectedMess, enc);
    ASSERT_TRUE(res == true);

    std::tie(enc, err) = AC.sign(expectedMess);
    ASSERT_TRUE(err == entity::ERR_OK);
    std::tie(res, err) = AS.verify(expectedMess, enc);
    ASSERT_TRUE(res == true);

    return 0;
}

int TestAES() {
    std::vector<uint8_t> key(sec::SYMMLEN / 8);
    RAND_bytes(key.data(), sec::SYMMLEN / 8);

    sec::SymCrypt SC(key);

    // test functionality
    auto [res, err] = SC.encrypt(expectedMess);
    ASSERT_TRUE(err == entity::ERR_OK);
    std::tie(res, err) = SC.decrypt(res);
    ASSERT_TRUE(err == entity::ERR_OK);

    ASSERT_TRUE(expectedMess == res);

    // test using sym encrypt on long vector
    auto encoded = std::vector<uint8_t>(3000);
    std::tie(res, err) = SC.encrypt(encoded);
    ASSERT_TRUE(err == entity::ERR_OK);

    return 0;
}

int TestHash() {
    std::string mess = "message";
    ASSERT_TRUE(sec::Hash(mess) == sec::Hash(mess));

    return 0;
}

int TestHashAndSalt() {
    std::string password = "secret";

    auto [hashandsalt, err] = sec::HashAndSalt(password);
    ASSERT_TRUE(err == entity::ERR_OK);

    ASSERT_TRUE(sec::VerifyHash(hashandsalt, password));

    return 0;
}

int TestMAC() {
    sec::Hmac h;
    ASSERT_TRUE(h.MAC(expectedMess) == h.MAC(expectedMess));

    return 0;
}

int TestEncodeEVP_PKEY() {
    EVP_PKEY *key = EVP_RSA_gen(1024);

    auto [encoded, err] = sec::encodePeerKey(key);
    ASSERT_TRUE(err == entity::ERR_OK);

    auto [decoded, err2] = sec::decodePeerKey(encoded);
    auto [encoded2, err3] = sec::encodePeerKey(decoded);

    return 0;
}