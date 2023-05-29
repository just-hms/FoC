#include "test.h"

int TestRepo() {
    std::cout << "lolz" << std::endl;

    repo::BankRepo r(DATA_PATH, "secret", 5);
    std::cout << "lulz" << std::endl;

    auto us1 = entity::User{
        .username="kek",
        .password="lolz",
    };
    std::cout << "kek" << std::endl;

    auto err = r.Create(&us1);
    ASSERT_TRUE(err == entity::ERR_OK);
    
    TEST_PASSED();
}