#include "test.h"

int TestRepo() {
    repo::BankRepo r(DATA_PATH, "secret", 5);

    auto us1 = entity::User{
        .username="kek",
        .password="lolz",
    };

    auto err = r.Create(&us1);
    ASSERT_TRUE(err == entity::ERR_OK);
    
    TEST_PASSED();
}