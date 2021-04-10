// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "common/assertion/include/exception.h"
#include "common/assertion/include/throw.h"

#include <stdexcept>
#include <string>

#include <boost/assert.hpp>
#include <boost/current_function.hpp>

#define MMOTD_THROW_ASSERTION(msg)                                                                                \
    mmotd::assertion::ThrowException(                                                                             \
        mmotd::assertion::Assertion(                                                                              \
            mmotd::assertion::MakeExceptionMessage("Assertion", msg, __FILE__, __LINE__, BOOST_CURRENT_FUNCTION), \
            true),                                                                                                \
        __FILE__,                                                                                                 \
        __LINE__,                                                                                                 \
        BOOST_CURRENT_FUNCTION)

#define MMOTD_THROW_INVALID_ARGUMENT(msg)                                                                 \
    mmotd::assertion::ThrowException(                                                                     \
        mmotd::assertion::InvalidArgument(mmotd::assertion::MakeExceptionMessage("InvalidArgument",       \
                                                                                 msg,                     \
                                                                                 __FILE__,                \
                                                                                 __LINE__,                \
                                                                                 BOOST_CURRENT_FUNCTION), \
                                          true),                                                          \
        __FILE__,                                                                                         \
        __LINE__,                                                                                         \
        BOOST_CURRENT_FUNCTION)

#define MMOTD_THROW_DOMAIN_ERROR(msg)                                                                               \
    mmotd::assertion::ThrowException(                                                                               \
        mmotd::assertion::DomainError(                                                                              \
            mmotd::assertion::MakeExceptionMessage("DomainError", msg, __FILE__, __LINE__, BOOST_CURRENT_FUNCTION), \
            true),                                                                                                  \
        __FILE__,                                                                                                   \
        __LINE__,                                                                                                   \
        BOOST_CURRENT_FUNCTION)

#define MMOTD_THROW_LENGTH_ERROR(msg)                                                                               \
    mmotd::assertion::ThrowException(                                                                               \
        mmotd::assertion::LengthError(                                                                              \
            mmotd::assertion::MakeExceptionMessage("LengthError", msg, __FILE__, __LINE__, BOOST_CURRENT_FUNCTION), \
            true),                                                                                                  \
        __FILE__,                                                                                                   \
        __LINE__,                                                                                                   \
        BOOST_CURRENT_FUNCTION)

#define MMOTD_THROW_OUT_OF_RANGE(msg)                                                                              \
    mmotd::assertion::ThrowException(                                                                              \
        mmotd::assertion::OutOfRange(                                                                              \
            mmotd::assertion::MakeExceptionMessage("OutOfRange", msg, __FILE__, __LINE__, BOOST_CURRENT_FUNCTION), \
            true),                                                                                                 \
        __FILE__,                                                                                                  \
        __LINE__,                                                                                                  \
        BOOST_CURRENT_FUNCTION)

#define MMOTD_THROW_RUNTIME_ERROR(msg)                                                                               \
    mmotd::assertion::ThrowException(                                                                                \
        mmotd::assertion::RuntimeError(                                                                              \
            mmotd::assertion::MakeExceptionMessage("RuntimeError", msg, __FILE__, __LINE__, BOOST_CURRENT_FUNCTION), \
            true),                                                                                                   \
        __FILE__,                                                                                                    \
        __LINE__,                                                                                                    \
        BOOST_CURRENT_FUNCTION)

#define MMOTD_THROW_RANGE_ERROR(msg)                                                                               \
    mmotd::assertion::ThrowException(                                                                              \
        mmotd::assertion::RangeError(                                                                              \
            mmotd::assertion::MakeExceptionMessage("RangeError", msg, __FILE__, __LINE__, BOOST_CURRENT_FUNCTION), \
            true),                                                                                                 \
        __FILE__,                                                                                                  \
        __LINE__,                                                                                                  \
        BOOST_CURRENT_FUNCTION)

#define MMOTD_THROW_OVERFLOW_ERROR(msg)                                                                               \
    mmotd::assertion::ThrowException(                                                                                 \
        mmotd::assertion::OverflowError(                                                                              \
            mmotd::assertion::MakeExceptionMessage("OverflowError", msg, __FILE__, __LINE__, BOOST_CURRENT_FUNCTION), \
            true),                                                                                                    \
        __FILE__,                                                                                                     \
        __LINE__,                                                                                                     \
        BOOST_CURRENT_FUNCTION)

#define MMOTD_THROW_UNDERFLOW_ERROR(msg)                                                                               \
    mmotd::assertion::ThrowException(                                                                                  \
        mmotd::assertion::UnderflowError(                                                                              \
            mmotd::assertion::MakeExceptionMessage("UnderflowError", msg, __FILE__, __LINE__, BOOST_CURRENT_FUNCTION), \
            true),                                                                                                     \
        __FILE__,                                                                                                      \
        __LINE__,                                                                                                      \
        BOOST_CURRENT_FUNCTION)

#define MMOTD_PRECONDITION BOOST_ASSERT
#define MMOTD_PRECONDITIONS(expr, msg) BOOST_ASSERT_MSG(expr, std::string(msg).c_str())
#define MMOTD_CHECK BOOST_ASSERT
#define MMOTD_CHECKS(expr, msg) BOOST_ASSERT_MSG(expr, std::string(msg).c_str())
#define MMOTD_ALWAYS_FAIL(msg) BOOST_ASSERT_MSG(false, std::string(msg).c_str())
//#define POSTCONDITION BOOST_ASSERT
//#define POSTCONDITION BOOST_ASSERT

namespace mmotd::assertion {

class Assertion : public std::runtime_error, public virtual boost::exception {
public:
    explicit Assertion(const char *message, bool includes_stack_trace = false);
    explicit Assertion(const std::string &message, bool includes_stack_trace = false);

    Assertion(const Assertion &) = default;
    Assertion &operator=(const Assertion &) = default;
    Assertion(Assertion &&) = default;
    Assertion &operator=(Assertion &&) = default;
    ~Assertion() noexcept override;
};

std::string
MakeExceptionMessage(const char *exception_type, const char *msg, const char *file, long line, const char *function);

std::string MakeExceptionMessage(const std::string &exception_type,
                                 const std::string &msg,
                                 const char *file,
                                 long line,
                                 const char *function);

std::string MakeExceptionMessage(const char *exception_type, const char *msg);

std::string MakeExceptionMessage(const std::string &exception_type, const std::string &msg);

} // namespace mmotd::assertion
