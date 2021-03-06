// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/assertion/include/exception.h"
#include "common/assertion/include/stack_trace.h"
#include "common/include/algorithm.h"
#include "common/include/logging.h"
#include "common/include/source_location_common.h"

#include <string_view>

#include <fmt/format.h>
#include <fmt/ostream.h>

using fmt::format;
using namespace std;
using mmotd::source_location::TrimFileName;
using mmotd::source_location::TrimFunction;

namespace {

static constexpr const char *NULLPTR_STR = "<nullptr>";
static constexpr const char *UNKNOWN_EXCEPTION_STR = "<unknownn exception>";

inline constexpr string_view GetMessage(const char *msg) noexcept {
    return msg == nullptr ? string_view{NULLPTR_STR} : string_view(msg);
}

inline constexpr string_view GetExceptionType(const char *exception_type) noexcept {
    return exception_type == nullptr ? string_view{UNKNOWN_EXCEPTION_STR} : string_view(exception_type);
}

inline string GetSourceLocation(string file_name, string function_name, long line) {
    auto source_location = string{};
    if (!empty(file_name)) {
        source_location = file_name;
    }
    if (!empty(function_name)) {
        source_location += format(FMT_STRING("{}{}"), (empty(source_location) ? "" : ":"), function_name);
    }
    if (line != 0) {
        source_location = format(FMT_STRING("{}@{}"), source_location, line);
    }
    return source_location;
}

inline string GetExceptionAndMessage(const char *exception_type_str, const char *msg_str) {
    auto exception_type = GetExceptionType(exception_type_str);
    auto msg = GetMessage(msg_str);
    if (!empty(exception_type) && !empty(msg)) {
        return format(FMT_STRING("{}: {}"), exception_type, msg);
    } else if (!empty(exception_type)) {
        return format(FMT_STRING("ERROR: {}"), exception_type);
    } else if (!empty(msg)) {
        return format(FMT_STRING("ERROR: {}"), msg);
    } else {
        return string{"<unknown error>"};
    }
}

string MakeFailedExpressionAssertionMessage(const char *expr,
                                            const char *msg,
                                            const char *file,
                                            long line,
                                            const char *function) {
    auto assert_msg = format(FMT_STRING("Assertion: {} located {}"),
                             expr,
                             GetSourceLocation(TrimFileName(file), TrimFunction(function), line));
    if (msg != nullptr) {
        assert_msg += format(FMT_STRING(", {}"), msg);
    }
    if (auto stacktrace = mmotd::assertion::GetStackTrace(); !empty(stacktrace)) {
        assert_msg += format(FMT_STRING("\n{}"), stacktrace);
    }
    return assert_msg;
}

string MakeExceptionMessageImpl(const char *exception_type,
                                const char *msg,
                                const char *file,
                                long line,
                                const char *function) {
    auto exception_msg = format(FMT_STRING("{}, {}"),
                                GetSourceLocation(TrimFileName(file), TrimFunction(function), line),
                                GetExceptionAndMessage(exception_type, msg));
    if (auto stacktrace = mmotd::assertion::GetStackTrace(); !empty(stacktrace)) {
        exception_msg += format(FMT_STRING("\n{}"), stacktrace);
    }
    return exception_msg;
}

string MakeExceptionMessageImpl(const char *exception_type, const char *msg) {
    auto exception_msg = GetExceptionAndMessage(exception_type, msg);
    if (auto stacktrace = mmotd::assertion::GetStackTrace(); !empty(stacktrace)) {
        exception_msg += format(FMT_STRING("\n{}"), stacktrace);
    }
    return exception_msg;
}

} // namespace

namespace boost {

void assertion_failed_msg(const char *expr, const char *msg, const char *function, const char *file, long line) {
    auto assert_msg = MakeFailedExpressionAssertionMessage(expr, msg, file, line, function);
    mmotd::assertion::ThrowException(mmotd::assertion::Assertion(assert_msg, true), file, line, function);
}

void assertion_failed(const char *expr, const char *function, const char *file, long line) {
    auto assert_msg = MakeFailedExpressionAssertionMessage(expr, nullptr, file, line, function);
    mmotd::assertion::ThrowException(mmotd::assertion::Assertion(assert_msg, true), file, line, function);
}

} // namespace boost

namespace mmotd::assertion {

string
MakeExceptionMessage(const char *exception_type, const char *msg, const char *file, long line, const char *function) {
    return MakeExceptionMessageImpl(exception_type, msg, file, line, function);
}

string MakeExceptionMessage(const string &exception_type,
                            const string &msg,
                            const char *file,
                            long line,
                            const char *function) {
    return MakeExceptionMessageImpl(exception_type.c_str(), msg.c_str(), file, line, function);
}

string MakeExceptionMessage(const char *exception_type, const char *msg) {
    return MakeExceptionMessageImpl(exception_type, msg);
}

string MakeExceptionMessage(const string &exception_type, const string &msg) {
    return MakeExceptionMessageImpl(exception_type.c_str(), msg.c_str());
}

Assertion::Assertion(const char *message, bool includes_stack_trace) :
    boost::exception(), runtime_error(includes_stack_trace ? message : MakeExceptionMessage("Assertion", message)) {
    LOG_ERROR("{}", what());
}

Assertion::Assertion(const string &message, bool includes_stack_trace) :
    boost::exception(),
    runtime_error(includes_stack_trace ? message : MakeExceptionMessage("Assertion", message.c_str())) {
    LOG_ERROR("{}", what());
}

Assertion::~Assertion() noexcept = default;

} // namespace mmotd::assertion
