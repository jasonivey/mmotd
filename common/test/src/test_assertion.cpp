#include "common/assertion/include/assertion.h"
#include "common/assertion/include/exception.h"
#include "common/include/source_location.h"
#include "common/test/include/exception_matcher.h"

#include <stdexcept>
#include <string_view>

#include <catch2/catch.hpp>
#include <fmt/format.h>

using namespace Catch;
using namespace Catch::Matchers;
using namespace mmotd::assertion;
using namespace std;

namespace {

void PreconditionSuccess() {
    PRECONDITIONS(true, "PRECONDITIONS does not throw when expression is true");
}

[[noreturn]] void PreconditionFail() {
    PRECONDITIONS(false, "PRECONDITIONS throws when expression is false");
}

void CheckSuccess() {
    CHECKS(true, "CHECKS does not throw when expression is true");
}

[[noreturn]] void CheckFail() {
    CHECKS(false, "CHECKS throws when expression is false");
}

void PostconditionSuccess() {
    POSTCONDITIONS(true, "POSTCONDITIONS does not throw when expression is true");
}

[[noreturn]] void PostconditionFail() {
    POSTCONDITIONS(false, "POSTCONDITIONS throws when expression is false");
}

} // namespace

namespace mmotd::test {

CATCH_TEST_CASE("Assertion inherits from RuntimeError", "[assertion]") {
    auto assertion = Assertion("assertion");
    CATCH_CHECK(dynamic_cast<const std::exception *>(&assertion) != nullptr);
    CATCH_CHECK(dynamic_cast<const std::runtime_error *>(&assertion) != nullptr);
}

CATCH_TEST_CASE("Assertion contains context", "[assertion]") {
    auto assertion =
        Assertion{GetExceptionMessage(mmotd::source_location::SourceLocation::current(), "Assertion", "assertion")};
    auto assertion_str = string(assertion.what());
    fmt::print(FMT_STRING("{}\n"), assertion_str);
    CATCH_CHECK_THAT(assertion_str, Contains("____C_A_T_C_H____T_E_S_T____"));
}

CATCH_TEST_CASE("ALWAYS_FAIL throws assertion exception", "[assertion]") {
    CATCH_CHECK_THROWS_MATCHES(ALWAYS_FAIL("throws an mmotd::assertion::Assertion"),
                               mmotd::assertion::Assertion,
                               MmotdExceptionMatcher("throws an mmotd::assertion::Assertion"));
}

CATCH_TEST_CASE("PRECONDITIONS(false) throws", "[assertion]") {
    CATCH_CHECK_THROWS_AS(PreconditionFail(), mmotd::assertion::Assertion);
}

CATCH_TEST_CASE("PRECONDITIONS(true) does not throw", "[assertion]") {
    CATCH_CHECK_NOTHROW(PreconditionSuccess());
}

CATCH_TEST_CASE("PRECONDITIONS(false) throws message", "[assertion]") {
    CATCH_CHECK_THROWS_MATCHES(PreconditionFail(),
                               mmotd::assertion::Assertion,
                               MmotdExceptionMatcher("PRECONDITIONS throws when expression is false"));
}

CATCH_TEST_CASE("CHECKS(false) throws", "[assertion]") {
    CATCH_CHECK_THROWS_AS(CheckFail(), mmotd::assertion::Assertion);
}

CATCH_TEST_CASE("CHECKS(true) does not throw", "[assertion]") {
    CATCH_CHECK_NOTHROW(CheckSuccess());
}

CATCH_TEST_CASE("CHECKS(false) throws message", "[assertion]") {
    CATCH_CHECK_THROWS_MATCHES(CheckFail(),
                               mmotd::assertion::Assertion,
                               MmotdExceptionMatcher("CHECKS throws when expression is false"));
}

CATCH_TEST_CASE("POSTCONDITIONS(false) throws", "[assertion]") {
    CATCH_CHECK_THROWS_AS(PostconditionFail(), mmotd::assertion::Assertion);
}

CATCH_TEST_CASE("POSTCONDITIONS(true) does not throw", "[assertion]") {
    CATCH_CHECK_NOTHROW(PostconditionSuccess());
}

CATCH_TEST_CASE("POSTCONDITIONS(false) throws message", "[assertion]") {
    CATCH_CHECK_THROWS_MATCHES(PostconditionFail(),
                               mmotd::assertion::Assertion,
                               MmotdExceptionMatcher("POSTCONDITIONS throws when expression is false"));
}

CATCH_TEST_CASE("THROW_ASSERTION does just that - throws", "[assertion]") {
    CATCH_CHECK_THROWS_MATCHES(THROW_ASSERTION("throws a std::runtime_error"),
                               mmotd::assertion::Assertion,
                               MmotdExceptionMatcher("throws a std::runtime_error"));
}

} // namespace mmotd::test
