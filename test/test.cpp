#include "test.h"

int main(){
    std::system(("mkdir -p " + DATA_PATH).c_str());
    
    return
    TestRawPingPong()   || 
    TestFunkyPingPong() ||
    TestDoubleFunky()   ||
    TestDH()            ||
    TestRSA()           ||
    TestAES()           ||
    TestHash()          ||
    TestHashAndSalt()   ||
    TestMAC()           ||
    TestEncodeEVP_PKEY()||
    TestRepo();
}