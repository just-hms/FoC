#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "./../src/entity/entity.h"
#include "./../src/router/router.h"
#include "./../src/network/network.h"
#include "./../src/config/config.h"
#include "./../src/security/security.h"
#include "./../src/protocol/protocol.h"
#include "./../src/repo/repo.h"

#define DATA_PATH std::string("./data/")

#define ASSERT_EQUAL(expected, actual) \
    if ((expected) != (actual)) { \
        std::cerr << "\033[31m" << __FUNCTION__ << " expected: " << (expected) << " got: " << (actual) << " at line " << __LINE__ << "\033[0m" << std::endl; \
        return 1; \
    }

#define ASSERT_TRUE(cond) \
    if (!(cond)) { \
        std::cerr << "\033[31m" << __FUNCTION__ << " expected: " << true << " got: " << false << " at line " << __LINE__ << "\033[0m" << std::endl; \
        return 1; \
    }

#define ASSERT_FALSE(cond) \
    if (cond) { \
        std::cerr << "\033[31m" << __FUNCTION__ << " expected: " << false << " got: " << true << " at line " << __LINE__ << "\033[0m" << std::endl; \
        return 1; \
    }

#define TEST_PASSED() \
    std::cout << __FUNCTION__ << " [OK]" << std::endl; \
    return 0;

int TestRawPingPong();
int TestFunkyPingPong();
int TestDoubleFunky();
int TestDH();
int TestRepo();
int TestRSA();
int TestAES();
int TestHash();
int TestHashAndSalt();
int TestMAC();
int TestEncodeEVP_PKEY();
