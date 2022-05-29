// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#define CATCH_CONFIG_RUNNER
#include "common/assertion/include/assertion.h"
#include "common/include/algorithm.h"
#include "common/include/logging.h"

#include <clocale>
#include <string_view>

#include <backward.hpp>
#include <catch2/catch.hpp>

using mmotd::algorithms::unused;
using std::string_view;

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "en_US.UTF-8");
    auto program_name = argv != nullptr && *argv != nullptr ? string_view(*argv) : string_view{};
    auto initilized = mmotd::logging::InitializeLogging(program_name);
    CHECKS(initilized, "unable to initialize logging");

    auto signal_handling = backward::SignalHandling{};
    unused(signal_handling);

    const auto new_severity = mmotd::logging::Severity::trace;
    const auto old_severity = SetSeverity(new_severity);
    LOG_INFO("set severity to '{}', previous severity '{}'", to_string(new_severity), to_string(old_severity));

    return Catch::Session().run(argc, argv);
}
