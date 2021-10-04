// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/assertion/include/throw.h"
#include "common/include/logging.h"
#include "common/include/source_location.h"

#include <exception>
#include <string>

#include <backward.hpp>
#include <boost/core/demangle.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/get_error_info.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>

using namespace std;
using namespace std::literals;
using mmotd::source_location::SourceLocation;

namespace {

SourceLocation GetExceptionLocation(const boost::exception &ex) noexcept {
    const auto *file_thrown = boost::get_error_info<boost::throw_file>(ex);
    const auto *function_thrown = boost::get_error_info<boost::throw_function>(ex);
    const auto *line_thrown = boost::get_error_info<boost::throw_line>(ex);
    if (file_thrown != nullptr && function_thrown != nullptr && line_thrown != nullptr) {
        return SourceLocation{*file_thrown, *function_thrown, *line_thrown, long{0}};
    } else {
        return SourceLocation{nullptr, nullptr, long{0}, long{0}};
    }
}

string GetExceptionBacktrace(const boost::exception &ex) noexcept {
    const auto *backtrace_thrown = boost::get_error_info<mmotd::assertion::throw_stack_trace>(ex);
    return backtrace_thrown != nullptr ? string{*backtrace_thrown} : string{};
}

string GetExceptionStr(const std::exception &ex) {
    if (const auto *what_msg = ex.what(); what_msg != nullptr) {
        return string{what_msg};
    } else {
        return boost::core::demangle(typeid(ex).name());
    }
}

bool ContainsBacktrace(const string &msg) {
    return msg.find("Stack trace (most recent call last):") != string::npos;
}

bool ContainsSourceLocation(const string &msg) {
    auto source_location = mmotd::source_location::from_string(msg, " at ", "\\n");
    return !empty(source_location);
}

string CreateExceptionMessage(const boost::exception &ex, const std::exception &std_ex) {
    auto msg = fmt::format(FMT_STRING("{}"), std::quoted(GetExceptionStr(std_ex)));
    if (!ContainsSourceLocation(msg)) {
        if (auto throw_location = GetExceptionLocation(ex); !throw_location.empty()) {
            msg += " at "s + to_string(throw_location);
        }
    }
    if (!ContainsBacktrace(msg)) {
        if (auto throw_backtrace = GetExceptionBacktrace(ex); !empty(throw_backtrace)) {
            return msg + "\n"s + throw_backtrace;
        }
    }
    return msg;
}

string CreateExceptionMessage(const std::exception &ex) {
    return GetExceptionStr(ex);
}

string CreateExceptionMessage() {
#if defined(__GLIBCXX__)
    if (auto *info = std::current_exception().__cxa_exception_type(); info != nullptr) {
        return string{boost::core::demangle(info->name())};
    }
#endif
    return "<unknown exception>"s;
}

string GetBoostExceptionMessageImpl(const boost::exception &ex,
                                    std::string_view context,
                                    const SourceLocation &source_location) {
    const auto *std_ex = dynamic_cast<const std::exception *>(&ex);
    CHECKS(std_ex != nullptr, "impossible, all boost::exceptions must inherit from std::exception");
    auto exception_msg = CreateExceptionMessage(ex, *std_ex);
    auto function_name = mmotd::source_location::function_name_to_string(source_location);
    if (empty(context)) {
        return fmt::format(FMT_STRING("caught boost::exception in {}: {}"), function_name, exception_msg);
    } else {
        return fmt::format(FMT_STRING("caught boost::exception {} in {}: {}"), context, function_name, exception_msg);
    }
}

string
GetStdExceptionMessageImpl(const std::exception &ex, std::string_view context, const SourceLocation &source_location) {
    const auto *boost_ex = dynamic_cast<const boost::exception *>(&ex);
    // CHECKS(boost_ex != nullptr, "yeah that's possible, someone didn't use the ThrowException mechanism");
    auto exception_msg = string{};
    if (boost_ex != nullptr) {
        exception_msg = CreateExceptionMessage(*boost_ex, ex);
    } else {
        exception_msg = CreateExceptionMessage(ex);
    }
    auto function_name = mmotd::source_location::function_name_to_string(source_location);
    if (empty(context)) {
        return fmt::format(FMT_STRING("caught std::exception in {}: {}"), function_name, exception_msg);
    } else {
        return fmt::format(FMT_STRING("caught std::exception {} in {}: {}"), context, function_name, exception_msg);
    }
}

string GetUnknownExceptionMessageImpl(std::string_view context, const SourceLocation &source_location) {
    auto exception_type = CreateExceptionMessage();
    auto function_name = mmotd::source_location::function_name_to_string(source_location);
    if (empty(context)) {
        return fmt::format(FMT_STRING("caught unknown exception in {}: {}"), function_name, exception_type);
    } else {
        return fmt::format(FMT_STRING("caught unknown exception {} in {}: {}"), context, function_name, exception_type);
    }
}

} // namespace

namespace mmotd::assertion {

string GetBoostExceptionMessage(const boost::exception &ex,
                                std::string_view context,
                                const SourceLocation &source_location) noexcept {
    try {
        return GetBoostExceptionMessageImpl(ex, context, source_location);
    } catch (...) { return string{}; }
}

string GetStdExceptionMessage(const std::exception &ex,
                              std::string_view context,
                              const SourceLocation &source_location) noexcept {
    try {
        return GetStdExceptionMessageImpl(ex, context, source_location);
    } catch (...) { return string{}; }
}

string GetUnknownExceptionMessage(std::string_view context, const SourceLocation &source_location) noexcept {
    try {
        return GetUnknownExceptionMessageImpl(context, source_location);
    } catch (...) { return string{}; }
}

} // namespace mmotd::assertion
