#include "test.h"

int main(){
    return 
    TestRawPingPong()   || 
    TestFunkyPingPong() ||
    TestDH()            ||
    TestRSA()           ||
    TestAES()           ||
    TestHash()          ||
    TestHashAndSalt()   ||
    TestMAC()           ||
    0;
}