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
        std::cerr << __FUNCTION__ << " expected: " << (expected) << " got: " << (actual) << " at line " << __LINE__ << std::endl; \
        return 1; \
    }

int TestRawPingPong();
int TestFunkyPingPong();
