// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "common/assertion/include/assertion.h"
#include "common/assertion/include/exception.h"
#include "common/assertion/include/stack_trace.h"
#include "common/include/source_location.h"

#include <string>

#include <backward.hpp>
#include <boost/exception/enable_error_info.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/exception.hpp>
// #include <boost/exception/get_error_info.hpp>
#include <boost/exception/info.hpp>
#include <boost/throw_exception.hpp>

namespace mmotd::assertion {

using throw_stack_trace = boost::error_info<struct tag_stacktrace, std::string>;

template<typename T>
void ThrowException(
    T &&ex,
    mmotd::source_location::SourceLocation source_location = mmotd::source_location::SourceLocation::current()) {
    ::boost::throw_exception(::boost::enable_error_info(ex)
                             << ::boost::throw_function(source_location.function_name())
                             << ::boost::throw_file(source_location.file_name())
                             << ::boost::throw_line(static_cast<int>(source_location.line()))
                             << throw_stack_trace(mmotd::assertion::GetStackTrace()));
}

template<typename T>
void ThrowException(
    const T &ex,
    mmotd::source_location::SourceLocation source_location = mmotd::source_location::SourceLocation::current()) {
    ::boost::throw_exception(::boost::enable_error_info(ex)
                             << ::boost::throw_function(source_location.function_name())
                             << ::boost::throw_file(source_location.file_name())
                             << ::boost::throw_line(static_cast<int>(source_location.line()))
                             << throw_stack_trace(mmotd::assertion::GetStackTrace()));
}

} // namespace mmotd::assertion
