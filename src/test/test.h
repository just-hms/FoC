#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#define ASSERT_EQUAL(expected, actual)                                 \
    if ((expected) != (actual)) {                                      \
        fprintf(stderr, "\033[31mTest failed at line %s:%d:\033[0m\n", \
                __FILE__, __LINE__);                                   \
        return 1;                                                      \
    }

#define ASSERT_TRUE(cond)                                              \
    if (!(cond)) {                                                     \
        fprintf(stderr, "\033[31mTest failed at line %s:%d:\033[0m\n", \
                __FILE__, __LINE__);                                   \
        return 1;                                                      \
    }

#define ASSERT_FALSE(cond)                                             \
    if (cond) {                                                        \
        fprintf(stderr, "\033[31mTest failed at line %s:%d:\033[0m\n", \
                __FILE__, __LINE__);                                   \
        return 1;                                                      \
    }
