// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/source_location.h"

#include <string>
#include <string_view>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

namespace mmotd::logging {

enum class Severity : int { trace = 0, debug = 1, info = 2, warn = 3, err = 4, critical = 5, off = 6 };
std::string to_string(Severity severity);

bool InitializeLogging(std::string_view binary_name);

Severity SetSeverity(Severity severity);

namespace detail {

void LogInternal(const mmotd::source_location::SourceLocation &source_location,
                 Severity severity,
                 const fmt::string_view &msg);

void LogInternal(const mmotd::source_location::SourceLocation &source_location,
                 Severity severity,
                 const fmt::string_view &msg,
                 fmt::format_args args);

} // namespace detail

template<typename S, typename... Args>
inline void
LogCommon(const mmotd::source_location::SourceLocation &location, Severity severity, const S &msg, Args &&...args) {
    detail::LogInternal(location, severity, msg, fmt::make_args_checked<Args...>(msg, args...));
}

template<typename S, typename... Args>
inline void
LogCommon(const mmotd::source_location::SourceLocation &location, Severity severity, const S *msg, Args &&...args) {
    detail::LogInternal(location, severity, std::string_view{msg}, fmt::make_args_checked<Args...>(msg, args...));
}

template<typename S>
inline void LogCommon(const mmotd::source_location::SourceLocation &location, Severity severity, const S &msg) {
    detail::LogInternal(location, severity, msg);
}

template<typename S>
inline void LogCommon(const mmotd::source_location::SourceLocation &location, Severity severity, const S *msg) {
    detail::LogInternal(location, severity, std::string_view{msg});
}

} // namespace mmotd::logging

#if defined(LOG_FATAL) || defined(LOG_ERROR) || defined(LOG_WARNING) || defined(LOG_INFO) || defined(LOG_DEBUG) ||     \
    defined(LOG_VERBOSE)
#error "LOG_* macros are already defined -- try including logging.h before any other headers."
#endif

// The following is part of C++17 'class template argument deduction'.  These objects are used to replace
//  the C-style macros which have long been associated with logging.  Instead the objects are used, along
//  with the 'user-defined deduction guides' following the definition of the 'LOG_' class templates,
//  to provide the same C-style macro functionality.
// See: https://en.cppreference.com/w/cpp/language/class_template_argument_deduction &
//      https://stackoverflow.com/a/57548488

// Note: for ease of use, the objects are defined in the global namespace.
// FIX_TODO: this may be worth enclosing in a namespace to avoid collisions and consistency.
template<typename... Args>
struct LOG_FATAL {
    LOG_FATAL(
        Args &&...args,
        const mmotd::source_location::SourceLocation &location = mmotd::source_location::SourceLocation::current()) {
        mmotd::logging::LogCommon(location, mmotd::logging::Severity::critical, std::forward<Args>(args)...);
    }
};

template<typename... Args>
struct LOG_ERROR {
    LOG_ERROR(
        Args &&...args,
        const mmotd::source_location::SourceLocation &location = mmotd::source_location::SourceLocation::current()) {
        mmotd::logging::LogCommon(location, mmotd::logging::Severity::err, std::forward<Args>(args)...);
    }
};

template<typename... Args>
struct LOG_WARNING {
    LOG_WARNING(
        Args &&...args,
        const mmotd::source_location::SourceLocation &location = mmotd::source_location::SourceLocation::current()) {
        mmotd::logging::LogCommon(location, mmotd::logging::Severity::warn, std::forward<Args>(args)...);
    }
};

template<typename... Args>
struct LOG_INFO {
    LOG_INFO(
        Args &&...args,
        const mmotd::source_location::SourceLocation &location = mmotd::source_location::SourceLocation::current()) {
        mmotd::logging::LogCommon(location, mmotd::logging::Severity::info, std::forward<Args>(args)...);
    }
};

template<typename... Args>
struct LOG_DEBUG {
    LOG_DEBUG(
        Args &&...args,
        const mmotd::source_location::SourceLocation &location = mmotd::source_location::SourceLocation::current()) {
        mmotd::logging::LogCommon(location, mmotd::logging::Severity::debug, std::forward<Args>(args)...);
    }
};

template<typename... Args>
struct LOG_VERBOSE {
    LOG_VERBOSE(
        Args &&...args,
        const mmotd::source_location::SourceLocation &location = mmotd::source_location::SourceLocation::current()) {
        mmotd::logging::LogCommon(location, mmotd::logging::Severity::trace, std::forward<Args>(args)...);
    }
};

template<typename... Args>
LOG_FATAL(Args &&...) -> LOG_FATAL<Args...>;

template<typename... Args>
LOG_ERROR(Args &&...) -> LOG_ERROR<Args...>;

template<typename... Args>
LOG_WARNING(Args &&...) -> LOG_WARNING<Args...>;

template<typename... Args>
LOG_INFO(Args &&...) -> LOG_INFO<Args...>;

template<typename... Args>
LOG_DEBUG(Args &&...) -> LOG_DEBUG<Args...>;

template<typename... Args>
LOG_VERBOSE(Args &&...) -> LOG_VERBOSE<Args...>;
