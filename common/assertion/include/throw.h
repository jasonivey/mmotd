// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/assertion/include/stack_trace.h"
#include "common/include/source_location.h"

#include <string>
#include <string_view>

#include <boost/exception/enable_error_info.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/throw_exception.hpp>

namespace mmotd::assertion {

using throw_stack_trace = boost::error_info<struct tag_stacktrace, std::string>;

template<typename T>
[[noreturn]] inline void ThrowException(T &&ex, mmotd::source_location::SourceLocation source_location) {
    ::boost::throw_exception(::boost::enable_error_info(ex)
                             << ::boost::throw_function(source_location.function_name())
                             << ::boost::throw_file(source_location.file_name())
                             << ::boost::throw_line(static_cast<int>(source_location.line()))
                             << throw_stack_trace(mmotd::assertion::GetStackTrace()));
}

template<typename T>
[[noreturn]] inline void ThrowException(const T &ex, mmotd::source_location::SourceLocation source_location) {
    ::boost::throw_exception(::boost::enable_error_info(ex)
                             << ::boost::throw_function(source_location.function_name())
                             << ::boost::throw_file(source_location.file_name())
                             << ::boost::throw_line(static_cast<int>(source_location.line()))
                             << throw_stack_trace(mmotd::assertion::GetStackTrace()));
}

std::string GetBoostExceptionMessage(const boost::exception &ex,
                                     std::string_view context = std::string_view{},
                                     const mmotd::source_location::SourceLocation &source_location =
                                         mmotd::source_location::SourceLocation::current()) noexcept;
std::string GetStdExceptionMessage(const std::exception &ex,
                                   std::string_view context = std::string_view{},
                                   const mmotd::source_location::SourceLocation &source_location =
                                       mmotd::source_location::SourceLocation::current()) noexcept;
std::string GetUnknownExceptionMessage(std::string_view context = std::string_view{},
                                       const mmotd::source_location::SourceLocation &source_location =
                                           mmotd::source_location::SourceLocation::current()) noexcept;

} // namespace mmotd::assertion
