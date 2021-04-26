#define CATCH_CONFIG_RUNNER
#include "common/include/logging.h"

#include <clocale>

#include <catch2/catch.hpp>

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "en_US.UTF-8");
    mmotd::logging::InitializeLogging(argv[0]);
    return Catch::Session().run(argc, argv);
}
