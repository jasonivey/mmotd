// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "apps/mmotd/include/cli_app_options_creator.h"
#include "apps/mmotd/include/main.h"
#include "common/include/algorithm.h"
#include "common/include/app_options.h"
#include "common/include/logging.h"
#include "common/results/include/output_template.h"
#include "common/results/include/output_template_printer.h"
#include "lib/include/computer_information.h"

#include <clocale>
#include <cstdlib>
#include <iostream>

#include <backward.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <fmt/format.h>

using mmotd::algorithms::unused;
using namespace fmt;
using namespace std;

namespace {

const tuple<bool, const AppOptions *> LoadAppOptions(const int argc, char **argv) {
    const auto *app_options_creator = CliAppOptionsCreator::ParseCommandLine(argc, argv);
    if (app_options_creator->IsAppFinished()) {
        return make_tuple(app_options_creator->IsErrorExit(), nullptr);
    }
    auto *app_options = AppOptions::Initialize(*app_options_creator);
    return make_tuple(false, app_options);
}

void PrintMmotd(const AppOptions &app_options) {
    auto options = app_options.Instance().GetOptions();
    if (!options.IsTemplatePathSet()) {
        LOG_ERROR("template file was not specified");
        return;
    }
    const auto &computer_information = mmotd::information::ComputerInformation::Instance();
    const auto &informations = computer_information.GetAllInformation();

    const auto template_filename = options.GetTemplatePath();
    if (auto output_template = mmotd::results::MakeOutputTemplate(template_filename); output_template) {
        mmotd::results::PrintOutputTemplate(*output_template, informations);
    }
}

int main_impl(int argc, char **argv) {
    setlocale(LC_ALL, "en_US.UTF-8");
    auto signal_handling = backward::SignalHandling{};
    unused(signal_handling);

    mmotd::logging::InitializeLogging(argv[0]);

    auto [error_encountered, app_options] = LoadAppOptions(argc, argv);
    if (app_options == nullptr) {
        return error_encountered ? EXIT_FAILURE : EXIT_SUCCESS;
    }
    if (app_options->GetOptions().IsLogSeveritySet()) {
        mmotd::logging::UpdateSeverity(app_options->GetOptions().GetLoggingSeverity());
    }

    PrintMmotd(*app_options);
    return EXIT_SUCCESS;
}

} // namespace

int main(int argc, char *argv[]) {
    auto retval = EXIT_SUCCESS;
    try {
        retval = main_impl(argc, argv);
    } catch (boost::exception &ex) {
        auto diag = boost::diagnostic_information(ex);
        LOG_FATAL("caught boost::exception in main: {}", diag);
        retval = EXIT_FAILURE;
    } catch (const std::exception &ex) {
        auto diag = boost::diagnostic_information(ex);
        LOG_FATAL("caught std::exception in main: {}", empty(diag) ? ex.what() : data(diag));
        retval = EXIT_FAILURE;
    } catch (...) {
        auto diag = boost::current_exception_diagnostic_information();
        LOG_FATAL("caught unknown exception in main: {}", diag);
        retval = EXIT_FAILURE;
    }
    return retval;
}
