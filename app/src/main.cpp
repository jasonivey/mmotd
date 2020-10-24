// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lastlog.h"
#include "network.h"
#include "external_network.h"
#include "system.h"
#include "app_options.h"
#include "logging.h"

#include "cli_app_options_creator.h"
#include "app_options.h"

#include <fmt/format.h>
#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>
#include <cstdlib>

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

    auto system_information = SystemInformation{};
    if (!system_information.TryDiscovery()) {
        cerr << "ERROR: unable to query for system information\n";
        return EXIT_FAILURE;
    } else {
        cout << "System Information: \n" << system_information << endl;
    }
    auto last_log = LastLog{};
    auto last_login_data = LastLoginData{};
    if (!last_log.GetLastLoginRecord(last_login_data)) {
        cerr << "ERROR: unable to query for the last login record\n";
        return EXIT_FAILURE;
    }
    auto network_info = NetworkInfo{};
    if (!network_info.TryDiscovery()) {
        cerr << "ERROR: unable to query for IP and mac address\n";
        return EXIT_FAILURE;
    } else {
        cout << "Network Info: \n" << to_string(network_info) << endl;
    }
    auto external_network = ExternalNetwork{};
    if (!external_network.TryDiscovery()) {
        cerr << "ERROR: unable to query for external IP address\n";
        return EXIT_FAILURE;
    } else {
        cout << "External IP: " << to_string(external_network) << endl;
    }
    return EXIT_SUCCESS;
}
