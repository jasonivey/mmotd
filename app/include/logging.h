// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <string>
#include <plog/Log.h>

namespace Logging {

const constexpr int CONSOLE_LOG = 1;

void DefaultInitializeLogging();
void InitializeLogging(const std::string &ini_file);
void UpdateSeverityFilter(int severity);

} // namespace Logging

#define MMOTD_LOG_VERBOSE                     PLOG(plog::verbose); PLOG(Logging::CONSOLE_LOG, plog::verbose)
#define MMOTD_LOG_DEBUG                       PLOG(plog::debug); PLOG(Logging::CONSOLE_LOG, plog::debug)
#define MMOTD_LOG_INFO                        PLOG(plog::info); PLOG(Logging::CONSOLE_LOG, plog::info)
#define MMOTD_LOG_WARNING                     PLOG(plog::warning); PLOG(Logging::CONSOLE_LOG, plog::warning)
#define MMOTD_LOG_ERROR                       PLOG(plog::error); PLOG(Logging::CONSOLE_LOG, plog::error)
#define MMOTD_LOG_FATAL                       PLOG(plog::fatal); PLOG(Logging::CONSOLE_LOG, plog::fatal)
#define MMOTD_LOG_NONE                        PLOG(plog::none); PLOG(Logging::CONSOLE_LOG, plog::none)
