// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/exception.h"

#include "common/assertion/include/stack_trace.h"
#include "common/include/logging.h"
#include "common/include/source_location.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>

#include <boost/exception/exception.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>

using namespace mmotd::source_location;
using namespace std;
using mmotd::logging::Severity;

namespace {

string GetExceptionMessageCommon(const SourceLocation &source_location,
                                 string_view exception_type,
                                 const fmt::string_view &format,
                                 fmt::format_args args) {
    using mmotd::logging::LogCommon;
    auto input_msg = fmt::vformat(format, args);
    auto msg = fmt::format(FMT_STRING("{}: {} at {}"), exception_type, input_msg, source_location);
    if (auto stacktrace = mmotd::assertion::GetStackTrace(); !empty(stacktrace)) {
        msg += fmt::format(FMT_STRING("\n{}"), stacktrace);
    }
    LogCommon(source_location, Severity::err, msg);
    return msg;
}

} // namespace

namespace mmotd::assertion {

string GetExceptionMessage(const mmotd::source_location::SourceLocation &source_location,
                           string_view exception_type,
                           const fmt::string_view &format_msg,
                           fmt::format_args args) {
    return GetExceptionMessageCommon(source_location, exception_type, format_msg, args);
}

string GetExceptionMessage(const mmotd::source_location::SourceLocation &source_location,
                           string_view exception_type,
                           const fmt::string_view &format_msg) {
    auto blank_args = fmt::format_args();
    return GetExceptionMessageCommon(source_location, exception_type, format_msg, blank_args);
}

InvalidArgument::InvalidArgument(const char *message) : boost::exception(), std::invalid_argument(message) {}

InvalidArgument::InvalidArgument(const std::string &message) : boost::exception(), std::invalid_argument(message) {}

InvalidArgument::~InvalidArgument() noexcept = default;

DomainError::DomainError(const char *message) : boost::exception(), std::domain_error(message) {}

DomainError::DomainError(const std::string &message) : boost::exception(), std::domain_error(message) {}

DomainError::~DomainError() noexcept = default;

LengthError::LengthError(const char *message) : boost::exception(), std::length_error(message) {}

LengthError::LengthError(const std::string &message) : boost::exception(), std::length_error(message) {}

LengthError::~LengthError() noexcept = default;

OutOfRange::OutOfRange(const char *message) : boost::exception(), std::out_of_range(message) {}

OutOfRange::OutOfRange(const std::string &message) : boost::exception(), std::out_of_range(message) {}

OutOfRange::~OutOfRange() noexcept = default;

RuntimeError::RuntimeError(const char *message) : boost::exception(), std::runtime_error(message) {}

RuntimeError::RuntimeError(const std::string &message) : boost::exception(), std::runtime_error(message) {}

RuntimeError::~RuntimeError() noexcept = default;

RangeError::RangeError(const char *message) : boost::exception(), std::range_error(message) {}

RangeError::RangeError(const std::string &message) : boost::exception(), std::range_error(message) {}

RangeError::~RangeError() noexcept = default;

OverflowError::OverflowError(const char *message) : boost::exception(), std::overflow_error(message) {}

OverflowError::OverflowError(const std::string &message) : boost::exception(), std::overflow_error(message) {}

OverflowError::~OverflowError() noexcept = default;

UnderflowError::UnderflowError(const char *message) : boost::exception(), std::underflow_error(message) {}

UnderflowError::UnderflowError(const std::string &message) : boost::exception(), std::underflow_error(message) {}

UnderflowError::~UnderflowError() noexcept = default;

Assertion::Assertion(const char *message) : boost::exception(), runtime_error(message) {}

Assertion::Assertion(const string &message) : boost::exception(), runtime_error(message) {}

Assertion::~Assertion() noexcept = default;

} // namespace mmotd::assertion
