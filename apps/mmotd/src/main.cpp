// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "apps/mmotd/include/cli_app_options_creator.h"
#include "apps/mmotd/include/main.h"
#include "common/include/app_options.h"
#include "common/include/logging.h"
#include "common/include/tty_template.h"
#include "common/include/tty_template_io.h"
#include "lib/include/computer_information.h"

#include <cstdlib>
#include <iostream>

#include <boost/exception/diagnostic_information.hpp>
#include <fmt/format.h>

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
        MMOTD_LOG_ERROR("template file was not specified");
        return;
    }
    const auto &computer_information = mmotd::information::ComputerInformation::Instance();
    const auto &informations = computer_information.GetAllInformation();

    const auto template_filename = options.GetTemplatePath();
    if (auto tty_template = mmotd::tty_template::MakeOutputTemplate(template_filename); tty_template) {
        mmotd::tty_template::PrintOutputTemplate(*tty_template, informations);
    }
}

} // namespace

int main(int argc, char *argv[]) {
    mmotd::logging::DefaultInitializeLogging("mmotd.log");

    auto [error_encountered, app_options] = LoadAppOptions(argc, argv);
    if (app_options == nullptr) {
        return error_encountered ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    auto retval = EXIT_SUCCESS;
    try {
        if (app_options->GetOptions().IsVerboseSet()) {
            mmotd::logging::UpdateSeverityFilter(app_options->GetOptions().GetVerbosityLevel());
        }
        PrintMmotd(*app_options);
    } catch (boost::exception &ex) {
        auto diag = boost::diagnostic_information(ex);
        auto error_str = format("caught boost::exception in main: {}", diag);
        PLOG_FATAL << error_str;
        std::cerr << error_str << std::endl;
        retval = EXIT_FAILURE;
    } catch (const std::exception &ex) {
        auto error_str = format("caught std::exception in main: {}", ex.what());
        PLOG_FATAL << error_str;
        std::cerr << error_str << std::endl;
        retval = EXIT_FAILURE;
    } catch (...) {
        auto diag = boost::current_exception_diagnostic_information();
        auto error_str = format("caught unknown exception in main: {}", diag);
        PLOG_FATAL << error_str;
        std::cerr << error_str << std::endl;
        retval = EXIT_FAILURE;
    }

    return retval;
}
