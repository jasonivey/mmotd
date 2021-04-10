#define CATCH_CONFIG_RUNNER
#include "common/include/logging.h"

#include <catch2/catch.hpp>

int main(int argc, char *argv[]) {
    mmotd::logging::DefaultInitializeLogging("mmotd_test.log");
    return Catch::Session().run(argc, argv);
}
