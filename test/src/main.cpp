#include "common/include/logging.h"

#include <gtest/gtest.h>

int main(int argc, char **argv) {
    mmotd::logging::DefaultInitializeLogging("mmotd_test.log");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
