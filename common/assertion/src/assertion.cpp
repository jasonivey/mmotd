// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/assertion/include/exception.h"
#include "common/assertion/include/stack_trace.h"
#include "common/include/algorithm.h"

#include <string_view>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

namespace {

static constexpr const char *UNKNOWN_FILE = "<file>";
static constexpr const char *UNKNOWN_FUNCTION = "<...>";
static constexpr const char *NULLPTR_STR = "<nullptr>";
static constexpr const char *UNKNOWN_EXCEPTION_STR = "<unknownn exception>";

inline constexpr string_view TrimFileName(const char *file) noexcept {
    auto file_str = file == nullptr ? string_view{UNKNOWN_FILE} : string_view(file);
    if (auto index = file_str.find_last_of('/'); index != string_view::npos) {
        file_str = file_str.substr(index + 1);
    }
    return file_str;
}

inline constexpr string_view TrimFunction(const char *function) noexcept {
    auto function_str = function == nullptr ? string_view{UNKNOWN_FUNCTION} : string_view(function);
    auto index = function_str.find('(');
    return function_str.substr(0, index);
}

inline constexpr string_view GetMessage(const char *msg) noexcept {
    return msg == nullptr ? string_view{NULLPTR_STR} : string_view(msg);
}

inline constexpr string_view GetExceptionType(const char *exception_type) noexcept {
    return exception_type == nullptr ? string_view{UNKNOWN_EXCEPTION_STR} : string_view(exception_type);
}

inline string GetSourceLocation(string_view file_name, string_view function_name, long line) {
    auto source_location = empty(file_name) ? string{} : string(data(file_name));
    if (!empty(function_name)) {
        source_location += empty(source_location) ? string(data(function_name)) : format(":{}", function_name);
    }
    if (empty(source_location) && line == 0) {
        return string{};
    } else {
        return format("{}@{}", source_location, line);
    }
}

inline string GetExceptionAndMessage(const char *exception_type_str, const char *msg_str) {
    auto exception_type = GetExceptionType(exception_type_str);
    auto msg = GetMessage(msg_str);
    if (!empty(exception_type) && !empty(msg)) {
        return format("{}: {}", exception_type, msg);
    } else if (!empty(exception_type)) {
        return format("ERROR: {}", exception_type);
    } else if (!empty(msg)) {
        return format("ERROR: {}", msg);
    } else {
        return string{"<unknown error>"};
    }
}

string MakeFailedExpressionAssertionMessage(const char *expr,
                                            const char *msg,
                                            const char *file,
                                            long line,
                                            const char *function) {
    auto assert_msg = format("{} -> Assertion: EXPR FAILED '{}'",
                             expr,
                             GetSourceLocation(TrimFileName(file), TrimFunction(function), line));
    if (msg != nullptr) {
        assert_msg += format(", {}", msg);
    }
    if (auto stacktrace = mmotd::assertion::GetStackTrace(); !empty(stacktrace)) {
        assert_msg += format("\n{}", stacktrace);
    }
    return assert_msg;
}

string MakeExceptionMessageImpl(const char *exception_type,
                                const char *msg,
                                const char *file,
                                long line,
                                const char *function) {
    auto exception_msg = format("{} -> {}",
                                GetSourceLocation(TrimFileName(file), TrimFunction(function), line),
                                GetExceptionAndMessage(exception_type, msg));
    if (auto stacktrace = mmotd::assertion::GetStackTrace(); !empty(stacktrace)) {
        exception_msg += format("\n{}", stacktrace);
    }
    return exception_msg;
}

string MakeExceptionMessageImpl(const char *exception_type, const char *msg) {
    auto exception_msg = GetExceptionAndMessage(exception_type, msg);
    if (auto stacktrace = mmotd::assertion::GetStackTrace(); !empty(stacktrace)) {
        exception_msg += format("\n{}", stacktrace);
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
    PLOG_ERROR << what();
}

Assertion::Assertion(const string &message, bool includes_stack_trace) :
    boost::exception(),
    runtime_error(includes_stack_trace ? message : MakeExceptionMessage("Assertion", message.c_str())) {
    PLOG_ERROR << what();
}

Assertion::~Assertion() noexcept {
}

} // namespace mmotd::assertion
