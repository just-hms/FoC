#include "test.h"

int main(){
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
    0;
}