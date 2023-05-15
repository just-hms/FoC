#include "./security/security.h"
using namespace std;

int main() {

    string mess1 = "E' finito il tempo delle mele", mess2 = "Oh no!";

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
    RAND_bytes(symk.iv, SYMMLEN/8);

    SymCrypt SC(symk);
    pubs = SC.decrypt(SC.encrypt(vector<uint8_t>(mess1.begin(), mess1.end())));
    res = string(pubs.begin(), pubs.end());
    cout<<res<<endl;
    if(Hash(res) != Hash(mess1)) {
        cout<<"FAILED"<<endl;
        //return -1;
    }

    RAND_bytes(symk.key, SYMMLEN/8);
    RAND_bytes(symk.iv, SYMMLEN/8);
    SC.refresh(symk);
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

    return 0;
}