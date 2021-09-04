// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "apps/mmotd/include/main.h"
#include "common/include/algorithm.h"
#include "common/include/cli_options_parser.h"
#include "common/include/config_options.h"
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
using mmotd::core::CliOptionsParser;
using mmotd::core::ConfigOptions;

namespace {

const tuple<bool, const CliOptionsParser *> LoadAppOptions(const int argc, char **argv) {
    const auto *parser = CliOptionsParser::ParseCommandLine(argc, argv);
    if (parser == nullptr) {
        return make_tuple(true, nullptr);
    } else if (parser->IsAppFinished()) {
        return make_tuple(parser->IsErrorExit(), nullptr);
    } else {
        return make_tuple(false, parser);
    }
}

void PrintMmotd(const CliOptionsParser &) {
    using namespace mmotd::results;

    const auto template_filename = ConfigOptions::Instance().GetValueAsStringOr("cli.template_path", string{});
    auto output_template = unique_ptr<OutputTemplate>{};
    if (!empty(template_filename)) {
        output_template = MakeOutputTemplate(template_filename);
    } else {
        output_template = MakeOutputTemplateFromDefault();
    }

    if (!output_template) {
        LOG_FATAL("unable to create output template from '{}'",
                  !empty(template_filename) ? template_filename : "<internal output template>");
        return;
    }

    const auto &computer_information = mmotd::information::ComputerInformation::Instance();
    const auto &informations = computer_information.GetAllInformation();

    PrintOutputTemplate(*output_template, informations);
}

void UpdateLogSeverity() {
    const auto log_severity_raw = ConfigOptions::Instance().GetValueAsIntegerOr("cli.log_severity", -1);
    if (log_severity_raw == -1) {
        return;
    }
    auto new_severity = static_cast<mmotd::logging::Severity>(log_severity_raw);
    auto current_severity = mmotd::logging::GetSeverity();
    if (new_severity != current_severity) {
        mmotd::logging::SetSeverity(new_severity);
    }
}

int main_impl(int argc, char **argv) {
    setlocale(LC_ALL, "en_US.UTF-8");
    auto signal_handling = backward::SignalHandling{};
    unused(signal_handling);

    mmotd::logging::InitializeLogging(*argv);

    auto [error_encountered, app_options] = LoadAppOptions(argc, argv);
    if (app_options == nullptr) {
        return error_encountered ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    UpdateLogSeverity();
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
