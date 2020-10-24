#include "logging.h"

#include <cassert>
#include <cstddef>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <utility>
#include <fmt/format.h>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/filter_parser.hpp>
#include <boost/log/utility/setup/settings.hpp>
#include <boost/log/utility/setup/from_settings.hpp>
#include <boost/log/utility/setup/from_stream.hpp>

namespace logging = boost::log;
using fmt::format;
using namespace std;

struct CoreLoggerSwitch {
    CoreLoggerSwitch(bool turn_off_logging = true) :
        logging_off_(turn_off_logging) {
        cout << format("INFO: CoreLoggerSwitch ctor turning logging {}\n", !logging_off_ ? "on" : "off");
        logging::core::get()->set_logging_enabled(!logging_off_);
    }
    ~CoreLoggerSwitch() {
        cout << format("INFO: CoreLoggerSwitch dtor turning logging {}\n", logging_off_ ? "on" : "off");
        logging::core::get()->set_logging_enabled(logging_off_);
    }
private:
    bool logging_off_ = true;
};

static void SetupLoggingFromFile(const string &ini_file) {
    assert (filesystem::exists(ini_file));
    cout << format("INFO: loading logging settings from {}\n", ini_file);

    auto ini_stream = ifstream(ini_file);
    auto core_logger_switch = CoreLoggerSwitch{}; (void) core_logger_switch;
    logging::core::get()->remove_all_sinks();
    logging::init_from_stream(ini_stream);
    logging::add_common_attributes();
    logging::core::get()->set_logging_enabled(true);
}

void Logging::DefaultInitializeLogging() {
    cout << "INFO: loading logging settings from hard-coded definitions\n";

    logging::settings setts;
    setts["Core"]["Filter"] = "%Severity% >= debug";
    setts["Core"]["DisableLogging"] = false;

    setts["Sinks.Console"]["Destination"] = "Console";
    setts["Sinks.Console"]["Filter"] = "%Severity% >= critical";
    setts["Sinks.Console"]["Format"] = "%TimeStamp% [%Severity%] %Message%";
    setts["Sinks.Console"]["AutoNewline"] = "InsertIfMissing";
    setts["Sinks.Console"]["Asynchronous"] = false;
    setts["Sinks.Console"]["AutoFlush"] = true;

    setts["Sinks.File"]["Destination"] = "TextFile";
    setts["Sinks.File"]["FileName"] = "mmotd_%3N.log";
    //setts["Sinks.File"]["Target"] = "/Users/jasoni/dev/";
    setts["Sinks.File"]["AutoFlush"] = true;
    setts["Sinks.File"]["AutoNewline"] = "InsertIfMissing";
    setts["Sinks.File"]["RotationSize"] = 10 * 1024 * 1024; // 10 MiB
    setts["Sinks.File"]["Format"] = "%TimeStamp% [%Severity%] %Message%";
    setts["Sinks.File"]["Append"] = true;

    logging::init_from_settings(setts);
    logging::add_common_attributes();
}

void Logging::InitializeLogging(const string &ini_file) {
    if (filesystem::exists(ini_file)) {
        SetupLoggingFromFile(ini_file);
    } else {
        cerr << format("ERROR: logging ini does not exist {}\n", ini_file);
        DefaultInitializeLogging();
    }
}

void Logging::UpdateSeverityFilter(int verbosity) {
    using namespace logging::trivial;
    auto core_logger_switch = CoreLoggerSwitch{}; (void) core_logger_switch;
    auto error_log_level = static_cast<int>(error);
    auto new_log_level = static_cast<severity_level>(std::max(error_log_level - verbosity, 0));
    auto filter_str = format("%Severity% >= {}", to_string(new_log_level));
    auto filter = logging::parse_filter(filter_str);
    logging::core::get()->set_filter(filter);
    //logging::core::get()->set_filter(logging::expressions::attr<boost::log::trivial::severity_level>("Severity") >= new_log_level);
    //logging::core::get()->set_filter(severity_level >= new_log_level);
}
