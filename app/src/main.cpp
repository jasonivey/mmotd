// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/app_options.h"
#include "lib/include/external_network.h"
#include "lib/include/lastlog.h"
#include "lib/include/network.h"
#include "lib/include/posix_system_information.h"

#include "view/include/computer_information_provider.h"
#include "view/include/computer_information_provider_factory.h"

#include "app/include/cli_app_options_creator.h"
#include "app/include/color.h"
#include "app/include/logging.h"

#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <fmt/format.h>
#include <iostream>

using fmt::format;
using namespace std;

static const AppOptions *load_app_options(const int argc, char **argv) {
    const auto *app_options_creator = CliAppOptionsCreator::ParseCommandLine(argc, argv);
    if (app_options_creator->IsAppFinished()) {
        return nullptr;
    }
    return AppOptions::Initialize(*app_options_creator);
}

int main(int argc, char *argv[]) {
    Logging::DefaultInitializeLogging();

    const AppOptions *app_options = load_app_options(argc, argv);
    if (app_options == nullptr) {
        return EXIT_FAILURE;
    }

    auto computer_information_providers = mmotd::GetComputerInformationProviders();
    (void) computer_information_providers;

    auto system_information = PosixSystemInformation{};
    if (!system_information.TryDiscovery()) {
        color::PrintError("unable to query for system information\n");
        return EXIT_FAILURE;
    } else {
        color::PrintInfo(format("system information:\n{}\n", to_string(system_information)));
    }
    auto last_log = LastLog{};
    auto last_login_data = LastLoginData{};
    if (!last_log.GetLastLoginRecord(last_login_data)) {
        color::PrintError("unable to query for the last login record\n");
        return EXIT_FAILURE;
    }
    auto network_info = NetworkInfo{};
    if (!network_info.TryDiscovery()) {
        color::PrintError("unable to query for IP and mac address\n");
        return EXIT_FAILURE;
    } else {
        color::PrintInfo(format("network info::\n{}\n", to_string(network_info)));
    }
    auto external_network = ExternalNetwork{};
    if (!external_network.TryDiscovery()) {
        color::PrintError("unable to query for external IP address\n");
        return EXIT_FAILURE;
    } else {
        color::PrintInfo(format("external ip: {}\n", to_string(external_network)));
    }
    return EXIT_SUCCESS;
}
