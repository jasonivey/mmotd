// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/computer_information_provider.h"
#include "view/include/computer_information_provider_factory.h"

#include "app/include/cli_app_options_creator.h"
#include "app/include/color.h"
#include "app/include/logging.h"
#include "app/include/main.h"

#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <fmt/color.h>
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

static void get_information(const AppOptions & /*app_options*/) {
    auto name_style = fmt::fg(fmt::color::green);
    auto information_style = fmt::fg(fmt::color::white) | fmt::emphasis::bold;
    auto providers = mmotd::GetComputerInformationProviders();
    fmt::print(name_style, "providers: ");
    fmt::print(information_style, "{}\n", providers.size());
    for (auto &&information_provider : mmotd::GetComputerInformationProviders()) {
        const auto &name = information_provider->GetInformationName();
        fmt::print(name_style, "{}: ", name);
        auto information = information_provider->GetComputerInformation();
        if (!information) {
            PLOG_ERROR << format("trying to query {}", name);
        }
        fmt::print(information_style, "{}\n", information.value_or(string{"none"}));
    }
}

int main(int argc, char *argv[]) {
    Logging::DefaultInitializeLogging();

    const AppOptions *app_options = load_app_options(argc, argv);
    if (app_options == nullptr) {
        return EXIT_FAILURE;
    }

    if (app_options->GetOptions().IsVerboseSet()) {
        cout << format("verbosity is set to {}\n", app_options->GetOptions().GetVerbosityLevel());
        Logging::UpdateSeverityFilter(app_options->GetOptions().GetVerbosityLevel());
    } else {
        cout << format("verbosity is set NOT to {}\n", app_options->GetOptions().GetVerbosityLevel());
    }

    get_information(*app_options);

    return EXIT_SUCCESS;
}
