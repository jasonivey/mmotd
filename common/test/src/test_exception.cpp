#include "common/assertion/include/exception.h"
#include "common/test/include/exception_matcher.h"

#include <stdexcept>
#include <string>

#include <catch2/catch.hpp>
#include <fmt/format.h>

using namespace Catch;
using namespace Catch::Matchers;
using namespace mmotd::assertion;
using namespace std;

namespace mmotd::test {

TEST_CASE("InvalidArgument inherits from std::invalid_argument", "[exception]") {
    auto ex = InvalidArgument("invalid argument");
    CHECK(dynamic_cast<const std::exception *>(&ex) != nullptr);
    CHECK(dynamic_cast<const std::invalid_argument *>(&ex) != nullptr);
    auto ex_str = string(ex.what());
    //fmt::print("{}\n", ex_str);
    CHECK_THAT(ex_str, Contains("mmotd::assertion::InvalidArgument::InvalidArgument"));
}

TEST_CASE("DomainError inherits from std::domain_error", "[exception]") {
    auto ex = DomainError("domain error");
    CHECK(dynamic_cast<const std::exception *>(&ex) != nullptr);
    CHECK(dynamic_cast<const std::domain_error *>(&ex) != nullptr);
    auto ex_str = string(ex.what());
    //fmt::print("{}\n", ex_str);
    CHECK_THAT(ex_str, Contains("mmotd::assertion::DomainError::DomainError"));
}

TEST_CASE("LengthError inherits from std::length_error", "[exception]") {
    auto ex = LengthError("length error");
    CHECK(dynamic_cast<const std::exception *>(&ex) != nullptr);
    CHECK(dynamic_cast<const std::length_error *>(&ex) != nullptr);
    auto ex_str = string(ex.what());
    //fmt::print("{}\n", ex_str);
    CHECK_THAT(ex_str, Contains("mmotd::assertion::LengthError::LengthError"));
}

TEST_CASE("OutOfRange inherits from std::out_of_range", "[exception]") {
    auto ex = OutOfRange("out of range");
    CHECK(dynamic_cast<const std::exception *>(&ex) != nullptr);
    CHECK(dynamic_cast<const std::out_of_range *>(&ex) != nullptr);
    auto ex_str = string(ex.what());
    //fmt::print("{}\n", ex_str);
    CHECK_THAT(ex_str, Contains("mmotd::assertion::OutOfRange::OutOfRange"));
}

TEST_CASE("RuntimeError inherits from std::runtime_error", "[exception]") {
    auto ex = RuntimeError("runtime error");
    CHECK(dynamic_cast<const std::exception *>(&ex) != nullptr);
    CHECK(dynamic_cast<const std::runtime_error *>(&ex) != nullptr);
    auto ex_str = string(ex.what());
    //fmt::print("{}\n", ex_str);
    CHECK_THAT(ex_str, Contains("mmotd::assertion::RuntimeError::RuntimeError"));
}

TEST_CASE("RangeError inherits from std::range_error", "[exception]") {
    auto ex = RangeError("range error");
    CHECK(dynamic_cast<const std::exception *>(&ex) != nullptr);
    CHECK(dynamic_cast<const std::range_error *>(&ex) != nullptr);
    auto ex_str = string(ex.what());
    //fmt::print("{}\n", ex_str);
    CHECK_THAT(ex_str, Contains("mmotd::assertion::RangeError::RangeError"));
}

TEST_CASE("OverflowError inherits from std::overflow_error", "[exception]") {
    auto ex = OverflowError("overflow error");
    CHECK(dynamic_cast<const std::exception *>(&ex) != nullptr);
    CHECK(dynamic_cast<const std::overflow_error *>(&ex) != nullptr);
    auto ex_str = string(ex.what());
    //fmt::print("{}\n", ex_str);
    CHECK_THAT(ex_str, Contains("mmotd::assertion::OverflowError::OverflowError"));
}

TEST_CASE("UnderflowError inherits from std::underflow_error", "[exception]") {
    auto ex = UnderflowError("underflow error");
    CHECK(dynamic_cast<const std::exception *>(&ex) != nullptr);
    CHECK(dynamic_cast<const std::underflow_error *>(&ex) != nullptr);
    auto ex_str = string(ex.what());
    //fmt::print("{}\n", ex_str);
    CHECK_THAT(ex_str, Contains("mmotd::assertion::UnderflowError::UnderflowError"));
}

TEST_CASE("MMOTD_THROW_INVALID_ARGUMENT throws", "[exception]") {
    CHECK_THROWS_MATCHES(MMOTD_THROW_INVALID_ARGUMENT("this should throw!"),
                         mmotd::assertion::InvalidArgument,
                         MmotdExceptionMatcher("this should throw!"));
}

TEST_CASE("MMOTD_THROW_DOMAIN_ERROR throws", "[exception]") {
    CHECK_THROWS_MATCHES(MMOTD_THROW_DOMAIN_ERROR("this should throw!"),
                         mmotd::assertion::DomainError,
                         MmotdExceptionMatcher("this should throw!"));
}

TEST_CASE("MMOTD_THROW_LENGTH_ERROR throws", "[exception]") {
    CHECK_THROWS_MATCHES(MMOTD_THROW_LENGTH_ERROR("this should throw!"),
                         mmotd::assertion::LengthError,
                         MmotdExceptionMatcher("this should throw!"));
}

TEST_CASE("MMOTD_THROW_OUT_OF_RANGE throws", "[exception]") {
    CHECK_THROWS_MATCHES(MMOTD_THROW_OUT_OF_RANGE("this should throw!"),
                         mmotd::assertion::OutOfRange,
                         MmotdExceptionMatcher("this should throw!"));
}

TEST_CASE("MMOTD_THROW_RUNTIME_ERROR throws", "[exception]") {
    CHECK_THROWS_MATCHES(MMOTD_THROW_RUNTIME_ERROR("this should throw!"),
                         mmotd::assertion::RuntimeError,
                         MmotdExceptionMatcher("this should throw!"));
}

TEST_CASE("MMOTD_THROW_RANGE_ERROR throws", "[exception]") {
    CHECK_THROWS_MATCHES(MMOTD_THROW_RANGE_ERROR("this should throw!"),
                         mmotd::assertion::RangeError,
                         MmotdExceptionMatcher("this should throw!"));
}

TEST_CASE("MMOTD_THROW_OVERFLOW_ERROR throws", "[exception]") {
    CHECK_THROWS_MATCHES(MMOTD_THROW_OVERFLOW_ERROR("this should throw!"),
                         mmotd::assertion::OverflowError,
                         MmotdExceptionMatcher("this should throw!"));
}

TEST_CASE("MMOTD_THROW_UNDERFLOW_ERROR throws", "[exception]") {
    CHECK_THROWS_MATCHES(MMOTD_THROW_UNDERFLOW_ERROR("this should throw!"),
                         mmotd::assertion::UnderflowError,
                         MmotdExceptionMatcher("this should throw!"));
}

} // namespace mmotd::test
