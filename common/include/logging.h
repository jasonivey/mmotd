// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/source_location.h"

#include <string>
#include <string_view>

#include <fmt/format.h>

namespace mmotd::logging {

enum class Severity { none = 0, fatal = 1, error = 2, warning = 3, info = 4, debug = 5, verbose = 6 };

Severity GetSeverity() noexcept;
void UpdateSeverity(Severity severity) noexcept;

void InitializeLogging(const std::string &binary_name);

void LogInternal(const mmotd::source_location::SourceLocation &source_location,
                 Severity severity,
                 const fmt::string_view &format,
                 fmt::format_args args);

void LogInternal(const mmotd::source_location::SourceLocation &source_location,
                 Severity severity,
                 const fmt::string_view &format);

template<typename S, typename... Args>
void Log(const mmotd::source_location::SourceLocation &source_location,
         Severity severity,
         const S &format,
         Args &&...args) {
    LogInternal(source_location, severity, format, fmt::make_args_checked<Args...>(format, args...));
}

template<typename S>
void Log(const mmotd::source_location::SourceLocation &source_location, Severity severity, const S &format) {
    LogInternal(source_location, severity, format);
}

} // namespace mmotd::logging

#define LOG_FATAL(format, ...)                                                     \
    do {                                                                           \
        if (mmotd::logging::GetSeverity() >= mmotd::logging::Severity::fatal) {    \
            mmotd::logging::Log(mmotd::source_location::SourceLocation::current(), \
                                mmotd::logging::Severity::fatal,                   \
                                FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__);    \
        }                                                                          \
    } while (false)
#define LOG_ERROR(format, ...)                                                     \
    do {                                                                           \
        if (mmotd::logging::GetSeverity() >= mmotd::logging::Severity::error) {    \
            mmotd::logging::Log(mmotd::source_location::SourceLocation::current(), \
                                mmotd::logging::Severity::error,                   \
                                FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__);    \
        }                                                                          \
    } while (false)
#define LOG_WARNING(format, ...)                                                   \
    do {                                                                           \
        if (mmotd::logging::GetSeverity() >= mmotd::logging::Severity::warning) {  \
            mmotd::logging::Log(mmotd::source_location::SourceLocation::current(), \
                                mmotd::logging::Severity::warning,                 \
                                FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__);    \
        }                                                                          \
    } while (false)
#define LOG_INFO(format, ...)                                                      \
    do {                                                                           \
        if (mmotd::logging::GetSeverity() >= mmotd::logging::Severity::info) {     \
            mmotd::logging::Log(mmotd::source_location::SourceLocation::current(), \
                                mmotd::logging::Severity::info,                    \
                                FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__);    \
        }                                                                          \
    } while (false)
#define LOG_DEBUG(format, ...)                                                     \
    do {                                                                           \
        if (mmotd::logging::GetSeverity() >= mmotd::logging::Severity::debug) {    \
            mmotd::logging::Log(mmotd::source_location::SourceLocation::current(), \
                                mmotd::logging::Severity::debug,                   \
                                FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__);    \
        }                                                                          \
    } while (false)
#define LOG_VERBOSE(format, ...)                                                   \
    do {                                                                           \
        if (mmotd::logging::GetSeverity() >= mmotd::logging::Severity::verbose) {  \
            mmotd::logging::Log(mmotd::source_location::SourceLocation::current(), \
                                mmotd::logging::Severity::verbose,                 \
                                FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__);    \
        }                                                                          \
    } while (false)
