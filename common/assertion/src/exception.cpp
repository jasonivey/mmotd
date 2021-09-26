// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/assertion/include/exception.h"
#include "common/include/algorithm.h"
#include "common/include/logging.h"

#include <stdexcept>

using namespace std;

namespace mmotd::assertion {

InvalidArgument::InvalidArgument(const char *message, bool includes_stack_trace) :
    boost::exception(),
    std::invalid_argument(includes_stack_trace ? message : MakeExceptionMessage("InvalidArgument", message)) {
    LOG_ERROR("{}", what());
}

InvalidArgument::InvalidArgument(const std::string &message, bool includes_stack_trace) :
    boost::exception(),
    std::invalid_argument(includes_stack_trace ? message : MakeExceptionMessage("InvalidArgument", message.c_str())) {
    LOG_ERROR("{}", what());
}

InvalidArgument::~InvalidArgument() noexcept = default;

DomainError::DomainError(const char *message, bool includes_stack_trace) :
    boost::exception(),
    std::domain_error(includes_stack_trace ? message : MakeExceptionMessage("DomainError", message)) {
    LOG_ERROR("{}", what());
}

DomainError::DomainError(const std::string &message, bool includes_stack_trace) :
    boost::exception(),
    std::domain_error(includes_stack_trace ? message : MakeExceptionMessage("DomainError", message.c_str())) {
    LOG_ERROR("{}", what());
}

DomainError::~DomainError() noexcept = default;

LengthError::LengthError(const char *message, bool includes_stack_trace) :
    boost::exception(),
    std::length_error(includes_stack_trace ? message : MakeExceptionMessage("LengthError", message)) {
    LOG_ERROR("{}", what());
}

LengthError::LengthError(const std::string &message, bool includes_stack_trace) :
    boost::exception(),
    std::length_error(includes_stack_trace ? message : MakeExceptionMessage("LengthError", message.c_str())) {
    LOG_ERROR("{}", what());
}

LengthError::~LengthError() noexcept = default;

OutOfRange::OutOfRange(const char *message, bool includes_stack_trace) :
    boost::exception(),
    std::out_of_range(includes_stack_trace ? message : MakeExceptionMessage("OutOfRangeError", message)) {
    LOG_ERROR("{}", what());
}

OutOfRange::OutOfRange(const std::string &message, bool includes_stack_trace) :
    boost::exception(),
    std::out_of_range(includes_stack_trace ? message : MakeExceptionMessage("OutOfRangeError", message.c_str())) {
    LOG_ERROR("{}", what());
}

OutOfRange::~OutOfRange() noexcept = default;

RuntimeError::RuntimeError(const char *message, bool includes_stack_trace) :
    boost::exception(),
    std::runtime_error(includes_stack_trace ? message : MakeExceptionMessage("RuntimeError", message)) {
    LOG_ERROR("{}", what());
}

RuntimeError::RuntimeError(const std::string &message, bool includes_stack_trace) :
    boost::exception(),
    std::runtime_error(includes_stack_trace ? message : MakeExceptionMessage("RuntimeError", message.c_str())) {
    LOG_ERROR("{}", what());
}

RuntimeError::~RuntimeError() noexcept = default;

RangeError::RangeError(const char *message, bool includes_stack_trace) :
    boost::exception(), std::range_error(includes_stack_trace ? message : MakeExceptionMessage("RangeError", message)) {
    LOG_ERROR("{}", what());
}

RangeError::RangeError(const std::string &message, bool includes_stack_trace) :
    boost::exception(),
    std::range_error(includes_stack_trace ? message : MakeExceptionMessage("RangeError", message.c_str())) {
    LOG_ERROR("{}", what());
}

RangeError::~RangeError() noexcept = default;

OverflowError::OverflowError(const char *message, bool includes_stack_trace) :
    boost::exception(),
    std::overflow_error(includes_stack_trace ? message : MakeExceptionMessage("OverflowError", message)) {
    LOG_ERROR("{}", what());
}

OverflowError::OverflowError(const std::string &message, bool includes_stack_trace) :
    boost::exception(),
    std::overflow_error(includes_stack_trace ? message : MakeExceptionMessage("OverflowError", message.c_str())) {
    LOG_ERROR("{}", what());
}

OverflowError::~OverflowError() noexcept = default;

UnderflowError::UnderflowError(const char *message, bool includes_stack_trace) :
    boost::exception(),
    std::underflow_error(includes_stack_trace ? message : MakeExceptionMessage("UnderflowError", message)) {
    LOG_ERROR("{}", what());
}

UnderflowError::UnderflowError(const std::string &message, bool includes_stack_trace) :
    boost::exception(),
    std::underflow_error(includes_stack_trace ? message : MakeExceptionMessage("UnderflowError", message.c_str())) {
    LOG_ERROR("{}", what());
}

UnderflowError::~UnderflowError() noexcept = default;

} // namespace mmotd::assertion
