#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "./../src/entity/entity.h"
#include "./../src/router/router.h"
#include "./../src/network/network.h"
#include "./../src/config/config.h"

#define ASSERT(expected, actual) \
    if ((expected) != (actual)) { \
        std::cerr << "\033[31m" << __FUNCTION__ << " expected: " << (expected) << " got: " << (actual) << " at line " << __LINE__ << "\033[0m" << std::endl; \
        return 1; \
    }

#define TEST_PASSED() \
    std::cout << __FUNCTION__ << " [OK]" << std::endl; \
    return 0;

int TestRawPingPong();
int TestFunkyPingPong();
