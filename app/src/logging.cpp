// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "app/include/logging.h"

#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Log.h>

#include <cassert>
#include <cstddef>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

using fmt::format;
using namespace std;

template<typename... Args>
inline void unused(Args &&...) {
}

plog::Severity gFileAppenderVerbosity = plog::verbose;
plog::Severity gConsoleAppenderVerbosity = plog::warning;

void Logging::DefaultInitializeLogging() {
    static auto file_appender = plog::RollingFileAppender<plog::TxtFormatter>("mmotd.log", 5 * 1048576, 3);
    static auto console_appender = plog::ColorConsoleAppender<plog::TxtFormatter>{};
    plog::init(gFileAppenderVerbosity, &file_appender);
    plog::init<CONSOLE_LOG>(gConsoleAppenderVerbosity, &console_appender);
}

void Logging::InitializeLogging(const string & /*ini_file*/) {
    DefaultInitializeLogging();
}

static plog::Severity convert_verbosity(int verbosity) {
    if (verbosity == 0) {
        return plog::none;
    }
    if (static_cast<plog::Severity>(verbosity + 3) > plog::verbose) {
        return plog::verbose;
    } else {
        return static_cast<plog::Severity>(verbosity + 3);
    }
}

void Logging::UpdateSeverityFilter(int verbosity) {
    plog::Severity new_severity = convert_verbosity(verbosity);
    auto new_file_severity = std::max(new_severity, gFileAppenderVerbosity);
    auto new_console_severity = std::max(new_severity, gConsoleAppenderVerbosity);
    if (gFileAppenderVerbosity != new_file_severity) {
        gFileAppenderVerbosity = new_file_severity;
        auto *file_log = plog::get();
        // cout << format("setting file log verbosity to {}\n", severityToString(gFileAppenderVerbosity));
        file_log->setMaxSeverity(gFileAppenderVerbosity);
    }
    if (gConsoleAppenderVerbosity != new_console_severity) {
        gConsoleAppenderVerbosity = new_console_severity;
        auto *console_log = plog::get<CONSOLE_LOG>();
        // cout << format("setting console log verbosity to {}\n", severityToString(gFileAppenderVerbosity));
        console_log->setMaxSeverity(gConsoleAppenderVerbosity);
    }
}
