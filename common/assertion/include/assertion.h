// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/assertion/include/exception.h"
#include "common/assertion/include/throw.h"
#include "common/include/source_location.h"

#include <stdexcept>
#include <string>
#include <string_view>

#include <boost/config.hpp> // for BOOST_LIKELY
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace mmotd::assertion {

template<typename S, typename... Args>
std::string MakeExceptionMessage(const mmotd::source_location::SourceLocation &source_location,
                                 std::string_view exception_type,
                                 const S &format,
                                 Args &&...args) {
    return mmotd::assertion::GetExceptionMessage(source_location,
                                                 exception_type,
                                                 format,
                                                 fmt::make_args_checked<Args...>(format, args...));
}

template<typename S>
std::string MakeExceptionMessage(const mmotd::source_location::SourceLocation &source_location,
                                 std::string_view exception_type,
                                 const S &format) {
    return mmotd::assertion::GetExceptionMessage(source_location, exception_type, format);
}

} // namespace mmotd::assertion

#define THROW_INVALID_ARGUMENT(format, ...)                                                                            \
    ::mmotd::assertion::ThrowException(::mmotd::assertion::InvalidArgument(::mmotd::assertion::MakeExceptionMessage(   \
                                           ::mmotd::source_location::SourceLocation::current(),                        \
                                           "InvalidArgument",                                                          \
                                           FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__)),                            \
                                       ::mmotd::source_location::SourceLocation::current())

#define THROW_DOMAIN_ERROR(format, ...)                                                                                \
    ::mmotd::assertion::ThrowException(::mmotd::assertion::DomainError(::mmotd::assertion::MakeExceptionMessage(       \
                                           ::mmotd::source_location::SourceLocation::current(),                        \
                                           "DomainError",                                                              \
                                           FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__)),                            \
                                       ::mmotd::source_location::SourceLocation::current())

#define THROW_LENGTH_ERROR(format, ...)                                                                                \
    ::mmotd::assertion::ThrowException(::mmotd::assertion::LengthError(::mmotd::assertion::MakeExceptionMessage(       \
                                           ::mmotd::source_location::SourceLocation::current(),                        \
                                           "LengthError",                                                              \
                                           FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__)),                            \
                                       ::mmotd::source_location::SourceLocation::current())

#define THROW_OUT_OF_RANGE(format, ...)                                                                                \
    ::mmotd::assertion::ThrowException(::mmotd::assertion::OutOfRange(::mmotd::assertion::MakeExceptionMessage(        \
                                           ::mmotd::source_location::SourceLocation::current(),                        \
                                           "OutOfRange",                                                               \
                                           FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__)),                            \
                                       ::mmotd::source_location::SourceLocation::current())

#define THROW_RUNTIME_ERROR(format, ...)                                                                               \
    ::mmotd::assertion::ThrowException(::mmotd::assertion::RuntimeError(::mmotd::assertion::MakeExceptionMessage(      \
                                           ::mmotd::source_location::SourceLocation::current(),                        \
                                           "RuntimeError",                                                             \
                                           FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__)),                            \
                                       ::mmotd::source_location::SourceLocation::current())

#define THROW_RANGE_ERROR(format, ...)                                                                                 \
    ::mmotd::assertion::ThrowException(::mmotd::assertion::RangeError(::mmotd::assertion::MakeExceptionMessage(        \
                                           ::mmotd::source_location::SourceLocation::current(),                        \
                                           "RangeError",                                                               \
                                           FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__)),                            \
                                       ::mmotd::source_location::SourceLocation::current())

#define THROW_OVERFLOW_ERROR(format, ...)                                                                              \
    ::mmotd::assertion::ThrowException(::mmotd::assertion::OverflowError(::mmotd::assertion::MakeExceptionMessage(     \
                                           ::mmotd::source_location::SourceLocation::current(),                        \
                                           "OverflowError",                                                            \
                                           FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__)),                            \
                                       ::mmotd::source_location::SourceLocation::current())

#define THROW_UNDERFLOW_ERROR(format, ...)                                                                             \
    ::mmotd::assertion::ThrowException(::mmotd::assertion::UnderflowError(::mmotd::assertion::MakeExceptionMessage(    \
                                           ::mmotd::source_location::SourceLocation::current(),                        \
                                           "UnderflowError",                                                           \
                                           FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__)),                            \
                                       ::mmotd::source_location::SourceLocation::current())

#define THROW_ASSERTION(format, ...)                                                                                   \
    ::mmotd::assertion::ThrowException(::mmotd::assertion::Assertion(::mmotd::assertion::MakeExceptionMessage(         \
                                           ::mmotd::source_location::SourceLocation::current(),                        \
                                           "Assertion",                                                                \
                                           FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__)),                            \
                                       ::mmotd::source_location::SourceLocation::current())

#define ALWAYS_FAIL(format, ...)                                                                                       \
    ::mmotd::assertion::ThrowException(::mmotd::assertion::Assertion(::mmotd::assertion::MakeExceptionMessage(         \
                                           ::mmotd::source_location::SourceLocation::current(),                        \
                                           "ALWAYS_FAIL",                                                              \
                                           FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__)),                            \
                                       ::mmotd::source_location::SourceLocation::current())

#define PRECONDITIONS(expr, format, ...)                                                                               \
    do {                                                                                                               \
        if (!(expr)) {                                                                                                 \
            ::mmotd::assertion::ThrowException(::mmotd::assertion::Assertion(::mmotd::assertion::MakeExceptionMessage( \
                                                   ::mmotd::source_location::SourceLocation::current(),                \
                                                   "PRECONDITIONS",                                                    \
                                                   FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__)),                    \
                                               ::mmotd::source_location::SourceLocation::current());                   \
        }                                                                                                              \
    } while (false)

#define CHECKS(expr, format, ...)                                                                                      \
    do {                                                                                                               \
        if (!(expr)) {                                                                                                 \
            ::mmotd::assertion::ThrowException(::mmotd::assertion::Assertion(::mmotd::assertion::MakeExceptionMessage( \
                                                   ::mmotd::source_location::SourceLocation::current(),                \
                                                   "CHECKS",                                                           \
                                                   FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__)),                    \
                                               ::mmotd::source_location::SourceLocation::current());                   \
        }                                                                                                              \
    } while (false)

#define POSTCONDITIONS(expr, format, ...)                                                                              \
    do {                                                                                                               \
        if (!(expr)) {                                                                                                 \
            ::mmotd::assertion::ThrowException(::mmotd::assertion::Assertion(::mmotd::assertion::MakeExceptionMessage( \
                                                   ::mmotd::source_location::SourceLocation::current(),                \
                                                   "POSTCONDITIONS",                                                   \
                                                   FMT_STRING(format) __VA_OPT__(, ) __VA_ARGS__)),                    \
                                               ::mmotd::source_location::SourceLocation::current());                   \
        }                                                                                                              \
    } while (false)
