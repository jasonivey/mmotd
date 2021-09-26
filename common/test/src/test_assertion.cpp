#include "common/assertion/include/assertion.h"
#include "common/test/include/exception_matcher.h"

#include <stdexcept>

#include <catch2/catch.hpp>
#include <fmt/format.h>

using namespace Catch;
using namespace Catch::Matchers;
using namespace mmotd::assertion;
using namespace std;

namespace mmotd::test {

TEST_CASE("Assertion inherits from RuntimeError", "[assertion]") {
    auto assertion = Assertion("assertion");
    CHECK(dynamic_cast<const std::exception *>(&assertion) != nullptr);
    CHECK(dynamic_cast<const std::runtime_error *>(&assertion) != nullptr);
}

TEST_CASE("Assertion contains context", "[assertion]") {
    auto assertion = Assertion("assertion");
    auto assertion_str = string(assertion.what());
    fmt::print(FMT_STRING("{}\n"), assertion_str);
    CHECK_THAT(assertion_str, Contains("mmotd::assertion::Assertion::Assertion"));
}

TEST_CASE("MMOTD_ALWAYS_FAIL throws catchable exception", "[assertion]") {
    CHECK_THROWS_MATCHES(MMOTD_ALWAYS_FAIL("throws a catchable exception"),
                         mmotd::assertion::Assertion,
                         MmotdExceptionMatcher("throws a catchable exception"));
}

TEST_CASE("MMOTD_PRECONDITION(true) does not throw", "[assertion]") {
    CHECK_NOTHROW(MMOTD_PRECONDITION(true));
}

TEST_CASE("MMOTD_PRECONDITION(false) throws", "[assertion]") {
    CHECK_THROWS_AS(MMOTD_PRECONDITION(false), mmotd::assertion::Assertion);
}

TEST_CASE("MMOTD_PRECONDITIONS(true) does not throw", "[assertion]") {
    CHECK_NOTHROW(MMOTD_PRECONDITIONS(true, "this should not throw!"));
}

TEST_CASE("MMOTD_PRECONDITIONS(false) throws", "[assertion]") {
    CHECK_THROWS_MATCHES(MMOTD_PRECONDITIONS(false, "this should throw!"),
                         mmotd::assertion::Assertion,
                         MmotdExceptionMatcher("this should throw!"));
}

TEST_CASE("MMOTD_CHECK(true) does not throw", "[assertion]") {
    CHECK_NOTHROW(MMOTD_CHECK(true));
}

TEST_CASE("MMOTD_CHECK(false) throws", "[assertion]") {
    CHECK_THROWS_AS(MMOTD_CHECK(false), mmotd::assertion::Assertion);
}

TEST_CASE("MMOTD_CHECKS(true) does not throw", "[assertion]") {
    CHECK_NOTHROW(MMOTD_CHECKS(true, "this should not throw!"));
}

TEST_CASE("MMOTD_CHECKS(false) throws", "[assertion]") {
    CHECK_THROWS_MATCHES(MMOTD_CHECKS(false, "this should throw!"),
                         mmotd::assertion::Assertion,
                         MmotdExceptionMatcher("this should throw!"));
}

TEST_CASE("MMOTD_THROW_ASSERTION throws", "[assertion]") {
    CHECK_THROWS_MATCHES(MMOTD_THROW_ASSERTION("this should throw!"),
                         mmotd::assertion::Assertion,
                         MmotdExceptionMatcher("this should throw!"));
}

} // namespace mmotd::test
