#define CATCH_CONFIG_RUNNER
#include "common/include/algorithm.h"
#include "common/include/logging.h"

#include <clocale>

#include <backward.hpp>
#include <catch2/catch.hpp>

using mmotd::algorithms::unused;

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "en_US.UTF-8");
    auto signal_handling = backward::SignalHandling{};
    unused(signal_handling);
    mmotd::logging::InitializeLogging(argv[0]);
    return Catch::Session().run(argc, argv);
}
