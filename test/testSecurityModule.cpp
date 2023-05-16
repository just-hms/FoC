#include "../src/security/security.h"
using namespace std;

int main() {

    string mess1 = "E' finito il tempo delle mele!!!", mess2 = "Oh no!";

    cout<<"DHKE test: ";
    EVP_PKEY *p, *sdh, *cdh;

    if(genDHparam(p) < 0) {
        cout<<"FAILED"<<endl;
        return -1;
    }
    if(genDH(sdh, p) < 0) {
        cout<<"FAILED"<<endl;
        return -1;
    }
    if(genDH(cdh, p) < 0) {
        cout<<"FAILED"<<endl;
        return -1;
    }
    if(derivateDH(sdh, cdh) != derivateDH(cdh, sdh)) {
        cout<<"FAILED"<<endl;
        return -1;
    }
    auto secret = derivateDH(sdh, cdh);
    SymCrypt R(keyFromSecret(string(secret.begin(), secret.end())));
    auto culo = R.decrypt(R.encrypt(vector<uint8_t>(mess1.begin(), mess1.end())));
    auto chiappe = string(culo.begin(), culo.end());
    if(Hash(chiappe) != Hash(mess1)) {
        cout<<"FAILED"<<endl;
        return -1;
    }

    EVP_PKEY_free(p);
    EVP_PKEY_free(sdh);
    EVP_PKEY_free(cdh);

    cout<<"PASSED"<<endl<<endl;

    cout<<"RSA generation test: ";

    if(generateRSAkeys(string("server"), string("server"), 4096) < 0) {
        cout<<"FAILED"<<endl;
        return -1;
    }
    if(generateRSAkeys(string("client"), string("client"), 4096) < 0) {
        cout<<"FAILED"<<endl;
        return -1;
    }
    cout<<"PASSED"<<endl<<endl;
    
    cout<<"AsymCrypt test: ";

    AsymCrypt AS("serverprivk.pem", "clientpubk.pem", "server");
    AsymCrypt AC("clientprivk.pem", "serverpubk.pem", "client");
    auto pubs = AC.decrypt(AS.encrypt(vector<uint8_t>(mess1.begin(), mess1.end())));
    string res(pubs.begin(), pubs.end());
    if(Hash(res) != Hash(mess1)) {
        cout<<"FAILED"<<endl;
        return -1;
    }
    pubs = AS.decrypt(AC.encrypt(vector<uint8_t>(mess2.begin(), mess2.end())));
    res = string(pubs.begin(), pubs.end());
    if(Hash(res) != Hash(mess2)) {
        cout<<"FAILED"<<endl;
        return -1;
    }
    cout<<"PASSED"<<endl<<endl;

    cout<<"SymCrypt test: ";
    sessionKey symk;
    RAND_bytes(symk.key, SYMMLEN/8);
    RAND_bytes(symk.iv, 16);

    SymCrypt SC(symk);
    pubs = SC.decrypt(SC.encrypt(vector<uint8_t>(mess1.begin(), mess1.end())));
    res = string(pubs.begin(), pubs.end());
    if(Hash(res) != Hash(mess1)) {
        cout<<"FAILED"<<endl;
        return -1;
    }

    RAND_bytes(symk.key, SYMMLEN/8);
    RAND_bytes(symk.iv, SYMMLEN/8);
    SC.refresh(symk);
    pubs = SC.decrypt(SC.encrypt(vector<uint8_t>(mess2.begin(), mess2.end())));
    res = string(pubs.begin(), pubs.end());
    if(Hash(res) != Hash(mess2)) {
        cout<<"FAILED"<<endl;
        return -1;
    }
    cout<<"PASSED"<<endl<<endl;

    cout<<"Hash test: ";
    if(Hash(mess1) != Hash(mess1)) {
        cout<<"FAILED"<<endl;
        return -1;
    }
    cout<<"PASSED"<<endl<<endl;

    cout<<"Hash and Salt test: ";
    if(HashAndSalt(mess1, mess2) != HashAndSalt(mess1, mess2)) {
        cout<<"FAILED"<<endl;
        return -1;
    }
    cout<<"PASSED"<<endl<<endl;

    cout<<"HMAC test: ";
    Hmac h("");
    if(h.MAC(mess1) != h.MAC(mess1)) {
        cout<<"FAILED"<<endl;
        return -1;
    }
    cout<<"PASSED"<<endl<<endl;

    return 0;
}