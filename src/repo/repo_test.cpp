#include "./../repo/repo.h"

#include "../test/test.h"
#include "./../config/config.h"
#include "./../entity/entity.h"
#include "./../uuid/uuid.h"

int TestRepo() {
    auto DATA_PATH = std::string("/tmp/test-repo/");
    std::system(("mkdir -p " + DATA_PATH + "transfers").c_str());
    defer { std::system(("rm -rf " + DATA_PATH).c_str()); };

    config::Config cfg;

    repo::BankRepo r(DATA_PATH, cfg.Secret, cfg.HistoryLen);

    auto us = entity::User{.username = "kek",
                           .password = "password",
                           .balance = entity::Balance{
                               .amount = 100,
                               .accountID = uuid::New(),
                           }};

    auto err = r.Create(&us);
    ASSERT_TRUE(err == entity::ERR_OK);

    us = entity::User{.username = "rospo",
                      .password = "password",
                      .balance = entity::Balance{
                          .amount = 70,
                          .accountID = uuid::New(),
                      }};
    err = r.Create(&us);
    ASSERT_TRUE(err == entity::ERR_OK);

    auto [_, errlogin] = r.Login("kek", "password");
    ASSERT_TRUE(errlogin == entity::ERR_OK);

    auto t = entity::Transaction{
        .from = "kek",
        .to = "rospo",
        .amount = 20,
    };
    auto [res, errTransfer] = r.Transfer(&t);
    ASSERT_TRUE(res);

    t = entity::Transaction{
        .from = "kek",
        .to = "rospo",
        .amount = 200000,
    };
    auto [res2, errTransfer2] = r.Transfer(&t);
    ASSERT_FALSE(res2);

    auto [history, errH] = r.History("kek");
    ASSERT_TRUE(errH == entity::ERR_OK);

    ASSERT_TRUE(history.size() == 1);
    ASSERT_TRUE(history[0].from == "kek");
    ASSERT_TRUE(history[0].to == "rospo");
    ASSERT_TRUE(history[0].amount == 20);

    auto [historyRospo, errRospo] = r.History("rospo");
    ASSERT_TRUE(errRospo == entity::ERR_OK);

    ASSERT_TRUE(historyRospo.size() == 1);
    ASSERT_TRUE(historyRospo[0].from == "kek");
    ASSERT_TRUE(historyRospo[0].to == "rospo");
    ASSERT_TRUE(historyRospo[0].amount == 20);

    auto [balance, errBalanceRospo] = r.Balance("rospo");
    ASSERT_TRUE(errBalanceRospo == entity::ERR_OK);
    ASSERT_TRUE(balance.amount == 90);

    return 0;
}