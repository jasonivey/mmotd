// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "common/assertion/include/stack_trace.h"

#include <string>

#include <backward.hpp>
#include <boost/exception/enable_error_info.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/exception.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/exception/info.hpp>
#include <boost/throw_exception.hpp>

namespace mmotd::assertion {

using throw_stack_trace = boost::error_info<struct tag_stacktrace, std::string>;

template<typename Exception>
void ThrowException(Exception &&ex, const char *file, long line, const char *function) {
    ::boost::throw_exception(::boost::enable_error_info(ex)
                             << ::boost::throw_function(function) << ::boost::throw_file(file)
                             << ::boost::throw_line(static_cast<int>(line))
                             << throw_stack_trace(mmotd::assertion::GetStackTrace()));
    //<< ::boost::throw_line(static_cast<int>(line)) << traced(boost::stacktrace::stacktrace()));
}

template<typename Exception>
void ThrowException(const Exception &ex, const char *file, long line, const char *function) {
    ::boost::throw_exception(::boost::enable_error_info(ex)
                             << ::boost::throw_function(function) << ::boost::throw_file(file)
                             << ::boost::throw_line(static_cast<int>(line))
                             << throw_stack_trace(mmotd::assertion::GetStackTrace()));
    //<< ::boost::throw_line(static_cast<int>(line)) << traced(boost::stacktrace::stacktrace()));
}

} // namespace mmotd::assertion
