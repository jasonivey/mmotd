// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "apps/mmotd/include/main.h"

#include "common/assertion/include/assertion.h"
#include "common/assertion/include/throw.h"
#include "common/include/algorithm.h"
#include "common/include/global_state.h"
#include "common/include/cli_options_parser.h"
#include "common/include/config_options.h"
#include "common/include/logging.h"
#include "common/include/output_template.h"
#include "common/include/output_template_writer.h"
#include "common/include/special_files.h"
#include "lib/include/computer_information.h"

#include <clocale>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <stdexcept>

#include <backward.hpp>
#include <boost/exception/exception.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>

using namespace fmt;
using namespace std;
using namespace std::string_literals;
using mmotd::algorithms::unused;
using mmotd::core::CliOptionsParser;
using mmotd::core::ConfigOptions;

namespace {

void PrintMmotd() {
    using namespace mmotd::output_template;
    using namespace mmotd::output_template_writer;
    using mmotd::core::special_files::ExpandEnvironmentVariables;

    auto template_filename = ConfigOptions::Instance().GetString("core.template_path"sv, ""sv);
    LOG_INFO("template file name: {}", quoted(empty(template_filename) ? "<builtin template>"s : template_filename));
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

    auto &computer_information = mmotd::information::ComputerInformation::Instance();
    auto informations = computer_information.GetAllInformations();

    auto writer = OutputTemplateWriter(output_template->GetColumns(), output_template->GetColumnItems(), informations);
    fmt::print(FMT_STRING("{}\n"), writer);
}

void UpdateLoggingDetails() {
    auto output_color = ConfigOptions::Instance().GetBoolean("logging.output_color"sv);
    if (output_color.has_value()) {
        mmotd::logging::SetColorOutput(*output_color);
    }
    auto flush_on_write = ConfigOptions::Instance().GetBoolean("logging.flush_on_write"sv);
    if (flush_on_write.has_value()) {
        mmotd::logging::SetFlushLogfileAfterEveryLine(*flush_on_write);
    }
    auto log_severity_raw = ConfigOptions::Instance().GetInteger("logging.severity"sv, -1);
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

    auto global_state = mmotd::globals::GlobalState{};
    unused(global_state);

    mmotd::logging::InitializeLogging(*argv);

    auto [app_finished, error_exit] = CliOptionsParser::ParseCommandLine(argc, argv);
    if (app_finished) {
        return error_exit ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    UpdateLoggingDetails();

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
