// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/logging.h"

#include <cassert>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include <fmt/format.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Severity.h>

using fmt::format;
using namespace std;

namespace mmotd::logging {

plog::Severity gFileSeverity = plog::verbose;
plog::Severity gConsoleSeverity = plog::warning;

void DefaultInitializeLogging(const string &filename) {
    static auto file_appender = plog::RollingFileAppender<plog::TxtFormatter>(filename.c_str(), 5 * 1048576, 3);
    static auto console_appender = plog::ColorConsoleAppender<plog::TxtFormatter>{};
    plog::init(gFileSeverity, &file_appender);
    plog::init<CONSOLE_LOG>(gConsoleSeverity, &console_appender);
}

// enum Severity
// {
//     none = 0,
//     fatal = 1,
//     error = 2,
//     warning = 3,
//     info = 4,
//     debug = 5,
//     verbose = 6
// };

// mapping from mmotd verbosity to plog severity modes
static inline constexpr plog::Severity convert_verbosity(Options::Verbosity verbosity) {
    if (verbosity == Options::Verbosity::Info) {
        return plog::info;
    } else if (verbosity == Options::Verbosity::Debug) {
        return plog::debug;
    } else if (verbosity == Options::Verbosity::Verbose) {
        return plog::verbose;
    } else {
        // Options::Verbosity::Off or Options::Verbosity::Inavlid
        return plog::none;
    }
}

void UpdateSeverityFilter(Options::Verbosity verbosity) {
    if (auto severity = std::max(convert_verbosity(verbosity), gFileSeverity); severity != gFileSeverity) {
        gFileSeverity = severity;
        auto *file_log = plog::get();
        file_log->setMaxSeverity(gFileSeverity);
    }
    if (auto severity = std::max(convert_verbosity(verbosity), gConsoleSeverity); severity != gConsoleSeverity) {
        gConsoleSeverity = severity;
        auto *console_log = plog::get<CONSOLE_LOG>();
        console_log->setMaxSeverity(gConsoleSeverity);
    }
}

} // namespace mmotd::logging
