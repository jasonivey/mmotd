// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "common/assertion/include/assertion.h"
#include "common/assertion/include/throw.h"

#include <exception>
#include <stdexcept>
#include <string>

#include <boost/exception/exception.hpp>

namespace mmotd::assertion {

// exception class to report invalid arguments
class InvalidArgument : public std::invalid_argument, public virtual boost::exception {
public:
    explicit InvalidArgument(const char *message, bool includes_stack_trace = false);
    explicit InvalidArgument(const std::string &message, bool includes_stack_trace = false);
    InvalidArgument(const InvalidArgument &) = default;
    InvalidArgument &operator=(const InvalidArgument &) = default;
    InvalidArgument(InvalidArgument &&) = default;
    InvalidArgument &operator=(InvalidArgument &&) = default;
    ~InvalidArgument() noexcept override;
};

// exception class to report domain errors
class DomainError : public std::domain_error, public virtual boost::exception {
public:
    explicit DomainError(const char *message, bool includes_stack_trace = false);
    explicit DomainError(const std::string &message, bool includes_stack_trace = false);
    DomainError(const DomainError &) = default;
    DomainError &operator=(const DomainError &) = default;
    DomainError(DomainError &&) = default;
    DomainError &operator=(DomainError &&) = default;
    ~DomainError() noexcept override;
};

// exception class to report attempts to exceed maximum allowed size
class LengthError : public std::length_error, public virtual boost::exception {
public:
    explicit LengthError(const char *message, bool includes_stack_trace = false);
    explicit LengthError(const std::string &message, bool includes_stack_trace = false);
    LengthError(const LengthError &) = default;
    LengthError &operator=(const LengthError &) = default;
    LengthError(LengthError &&) = default;
    LengthError &operator=(LengthError &&) = default;
    ~LengthError() noexcept override;
};

// exception class to report arguments outside of expected range
class OutOfRange : public std::out_of_range, public virtual boost::exception {
public:
    explicit OutOfRange(const char *message, bool includes_stack_trace = false);
    explicit OutOfRange(const std::string &message, bool includes_stack_trace = false);
    OutOfRange(const OutOfRange &) = default;
    OutOfRange &operator=(const OutOfRange &) = default;
    OutOfRange(OutOfRange &&) = default;
    OutOfRange &operator=(OutOfRange &&) = default;
    ~OutOfRange() noexcept override;
};

// exception class to indicate conditions only detectable at run time
class RuntimeError : public std::runtime_error, public virtual boost::exception {
public:
    explicit RuntimeError(const char *message, bool includes_stack_trace = false);
    explicit RuntimeError(const std::string &message, bool includes_stack_trace = false);
    RuntimeError(const RuntimeError &) = default;
    RuntimeError &operator=(const RuntimeError &) = default;
    RuntimeError(RuntimeError &&) = default;
    RuntimeError &operator=(RuntimeError &&) = default;
    ~RuntimeError() noexcept override;
};

// exception class to report range errors in internal computations
class RangeError : public std::range_error, public virtual boost::exception {
public:
    explicit RangeError(const char *message, bool includes_stack_trace = false);
    explicit RangeError(const std::string &message, bool includes_stack_trace = false);
    RangeError(const RangeError &) = default;
    RangeError &operator=(const RangeError &) = default;
    RangeError(RangeError &&) = default;
    RangeError &operator=(RangeError &&) = default;
    ~RangeError() noexcept override;
};

// exception class to report arithmetic overflows
class OverflowError : public std::overflow_error, public virtual boost::exception {
public:
    explicit OverflowError(const char *message, bool includes_stack_trace = false);
    explicit OverflowError(const std::string &message, bool includes_stack_trace = false);
    OverflowError(const OverflowError &) = default;
    OverflowError &operator=(const OverflowError &) = default;
    OverflowError(OverflowError &&) = default;
    OverflowError &operator=(OverflowError &&) = default;
    ~OverflowError() noexcept override;
};

// exception class to report arithmetic underflows
class UnderflowError : public std::underflow_error, public virtual boost::exception {
public:
    explicit UnderflowError(const char *message, bool includes_stack_trace = false);
    explicit UnderflowError(const std::string &message, bool includes_stack_trace = false);
    UnderflowError(const UnderflowError &) = default;
    UnderflowError &operator=(const UnderflowError &) = default;
    UnderflowError(UnderflowError &&) = default;
    UnderflowError &operator=(UnderflowError &&) = default;
    ~UnderflowError() noexcept override;
};

} // namespace mmotd::assertion
