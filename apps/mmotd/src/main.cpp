// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "apps/mmotd/include/main.h"
#include "common/assertion/include/assertion.h"
#include "common/assertion/include/throw.h"
#include "common/include/algorithm.h"
#include "common/include/cli_options_parser.h"
#include "common/include/config_options.h"
#include "common/include/logging.h"
#include "common/include/special_files.h"
#include "common/results/include/output_template.h"
#include "common/results/include/output_template_printer.h"
#include "lib/include/computer_information.h"

#include <clocale>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include <backward.hpp>
#include <boost/exception/exception.hpp>
#include <fmt/format.h>

using namespace fmt;
using namespace std;
using mmotd::algorithms::unused;
using mmotd::core::CliOptionsParser;
using mmotd::core::ConfigOptions;

namespace {

void PrintMmotd() {
    using namespace mmotd::results;
    using mmotd::core::special_files::ExpandEnvironmentVariables;

    const auto template_filename = ConfigOptions::Instance().GetValueAsStringOr("template_path", string{});
    LOG_INFO("template file name: {}", quoted(template_filename));
    auto output_template = unique_ptr<OutputTemplate>{};
    if (!empty(template_filename)) {
        output_template = MakeOutputTemplate(ExpandEnvironmentVariables(template_filename));
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

void UpdateLoggingFlushing() {
    auto logging_flush_value = ConfigOptions::Instance().GetValueAsBoolean("logging_flush");
    if (logging_flush_value.has_value()) {
        mmotd::logging::SetFlushLogfileAfterEveryLine(logging_flush_value.value());
    }
}

void UpdateLoggingSeverity() {
    const auto log_severity_raw = ConfigOptions::Instance().GetValueAsIntegerOr("log_severity", -1);
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
    auto signal_handling = backward::SignalHandling();
    unused(signal_handling);

    mmotd::logging::InitializeLogging(*argv);

    auto [app_finished, error_exit] = CliOptionsParser::ParseCommandLine(argc, argv);
    if (app_finished) {
        return error_exit ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    UpdateLoggingFlushing();
    UpdateLoggingSeverity();

    PrintMmotd();
    return EXIT_SUCCESS;
}

} // namespace

int main(int argc, char *argv[]) {
    auto retval = EXIT_SUCCESS;
    auto exception_message = string{};

    try {
        retval = main_impl(argc, argv);
    } catch (boost::exception &ex) {
        exception_message = mmotd::assertion::GetBoostExceptionMessage(ex);
    } catch (const std::exception &ex) {
        exception_message = mmotd::assertion::GetStdExceptionMessage(ex);
    } catch (...) { exception_message = mmotd::assertion::GetUnknownExceptionMessage(); }

    if (!empty(exception_message)) {
        LOG_FATAL("{}", exception_message);
        retval = EXIT_FAILURE;
    }
    return retval;
}
