// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
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

CATCH_TEST_CASE("InvalidArgument inherits from std::invalid_argument", "[exception]") {
    auto ex = InvalidArgument("invalid argument");
    CATCH_CHECK(dynamic_cast<const std::exception *>(&ex) != nullptr);
    CATCH_CHECK(dynamic_cast<const std::invalid_argument *>(&ex) != nullptr);
}

CATCH_TEST_CASE("InvalidArgument contains context", "[exception]") {
    auto ex = InvalidArgument{
        GetExceptionMessage(mmotd::source_location::SourceLocation::current(), "InvalidArgument", "InvalidArgument")};
    auto ex_str = string(ex.what());
    fmt::print(FMT_STRING("{}\n"), ex_str);
    CATCH_CHECK_THAT(ex_str, Contains("____C_A_T_C_H____T_E_S_T____"));
}

CATCH_TEST_CASE("DomainError inherits from std::domain_error", "[exception]") {
    auto ex = DomainError("domain error");
    CATCH_CHECK(dynamic_cast<const std::exception *>(&ex) != nullptr);
    CATCH_CHECK(dynamic_cast<const std::domain_error *>(&ex) != nullptr);
}

CATCH_TEST_CASE("DomainError contains context", "[exception]") {
    auto ex = DomainError{
        GetExceptionMessage(mmotd::source_location::SourceLocation::current(), "DomainError", "DomainError")};
    auto ex_str = string(ex.what());
    fmt::print(FMT_STRING("{}\n"), ex_str);
    CATCH_CHECK_THAT(ex_str, Contains("____C_A_T_C_H____T_E_S_T____"));
}

CATCH_TEST_CASE("LengthError inherits from std::length_error", "[exception]") {
    auto ex = LengthError("length error");
    CATCH_CHECK(dynamic_cast<const std::exception *>(&ex) != nullptr);
    CATCH_CHECK(dynamic_cast<const std::length_error *>(&ex) != nullptr);
}

CATCH_TEST_CASE("LengthError contains context", "[exception]") {
    auto ex = LengthError{
        GetExceptionMessage(mmotd::source_location::SourceLocation::current(), "LengthError", "LengthError")};
    auto ex_str = string(ex.what());
    fmt::print(FMT_STRING("{}\n"), ex_str);
    CATCH_CHECK_THAT(ex_str, Contains("____C_A_T_C_H____T_E_S_T____"));
}

CATCH_TEST_CASE("OutOfRange inherits from std::out_of_range", "[exception]") {
    auto ex = OutOfRange("out of range");
    CATCH_CHECK(dynamic_cast<const std::exception *>(&ex) != nullptr);
    CATCH_CHECK(dynamic_cast<const std::out_of_range *>(&ex) != nullptr);
}

CATCH_TEST_CASE("OutOfRange contains context", "[exception]") {
    auto ex =
        OutOfRange{GetExceptionMessage(mmotd::source_location::SourceLocation::current(), "OutOfRange", "OutOfRange")};
    auto ex_str = string(ex.what());
    fmt::print(FMT_STRING("{}\n"), ex_str);
    CATCH_CHECK_THAT(ex_str, Contains("____C_A_T_C_H____T_E_S_T____"));
}

CATCH_TEST_CASE("RuntimeError inherits from std::runtime_error", "[exception]") {
    auto ex = RuntimeError("runtime error");
    CATCH_CHECK(dynamic_cast<const std::exception *>(&ex) != nullptr);
    CATCH_CHECK(dynamic_cast<const std::runtime_error *>(&ex) != nullptr);
}

CATCH_TEST_CASE("RuntimeError contains context", "[exception]") {
    auto ex = RuntimeError{
        GetExceptionMessage(mmotd::source_location::SourceLocation::current(), "RuntimeError", "RuntimeError")};
    auto ex_str = string(ex.what());
    fmt::print(FMT_STRING("{}\n"), ex_str);
    CATCH_CHECK_THAT(ex_str, Contains("____C_A_T_C_H____T_E_S_T____"));
}

CATCH_TEST_CASE("RangeError inherits from std::range_error", "[exception]") {
    auto ex = RangeError("range error");
    CATCH_CHECK(dynamic_cast<const std::exception *>(&ex) != nullptr);
    CATCH_CHECK(dynamic_cast<const std::range_error *>(&ex) != nullptr);
}

CATCH_TEST_CASE("RangeError contains context", "[exception]") {
    auto ex =
        RangeError{GetExceptionMessage(mmotd::source_location::SourceLocation::current(), "RangeError", "RangeError")};
    auto ex_str = string(ex.what());
    fmt::print(FMT_STRING("{}\n"), ex_str);
    CATCH_CHECK_THAT(ex_str, Contains("____C_A_T_C_H____T_E_S_T____"));
}

CATCH_TEST_CASE("OverflowError inherits from std::overflow_error", "[exception]") {
    auto ex = OverflowError("overflow error");
    CATCH_CHECK(dynamic_cast<const std::exception *>(&ex) != nullptr);
    CATCH_CHECK(dynamic_cast<const std::overflow_error *>(&ex) != nullptr);
}

CATCH_TEST_CASE("OverflowError contains context", "[exception]") {
    auto ex = OverflowError{
        GetExceptionMessage(mmotd::source_location::SourceLocation::current(), "OverflowError", "OverflowError")};
    auto ex_str = string(ex.what());
    fmt::print(FMT_STRING("{}\n"), ex_str);
    CATCH_CHECK_THAT(ex_str, Contains("____C_A_T_C_H____T_E_S_T____"));
}

CATCH_TEST_CASE("UnderflowError inherits from std::underflow_error", "[exception]") {
    auto ex = UnderflowError("underflow error");
    CATCH_CHECK(dynamic_cast<const std::exception *>(&ex) != nullptr);
    CATCH_CHECK(dynamic_cast<const std::underflow_error *>(&ex) != nullptr);
}

CATCH_TEST_CASE("UnderflowError contains context", "[exception]") {
    auto ex = UnderflowError{
        GetExceptionMessage(mmotd::source_location::SourceLocation::current(), "UnderflowError", "UnderflowError")};
    auto ex_str = string(ex.what());
    fmt::print(FMT_STRING("{}\n"), ex_str);
    CATCH_CHECK_THAT(ex_str, Contains("____C_A_T_C_H____T_E_S_T____"));
}

CATCH_TEST_CASE("THROW_INVALID_ARGUMENT throws", "[exception]") {
    CATCH_CHECK_THROWS_MATCHES(THROW_INVALID_ARGUMENT("this should throw!"),
                               mmotd::assertion::InvalidArgument,
                               MmotdExceptionMatcher("this should throw!"));
}

CATCH_TEST_CASE("THROW_DOMAIN_ERROR throws", "[exception]") {
    CATCH_CHECK_THROWS_MATCHES(THROW_DOMAIN_ERROR("this should throw!"),
                               mmotd::assertion::DomainError,
                               MmotdExceptionMatcher("this should throw!"));
}

CATCH_TEST_CASE("THROW_LENGTH_ERROR throws", "[exception]") {
    CATCH_CHECK_THROWS_MATCHES(THROW_LENGTH_ERROR("this should throw!"),
                               mmotd::assertion::LengthError,
                               MmotdExceptionMatcher("this should throw!"));
}

CATCH_TEST_CASE("THROW_OUT_OF_RANGE throws", "[exception]") {
    CATCH_CHECK_THROWS_MATCHES(THROW_OUT_OF_RANGE("this should throw!"),
                               mmotd::assertion::OutOfRange,
                               MmotdExceptionMatcher("this should throw!"));
}

CATCH_TEST_CASE("THROW_RUNTIME_ERROR throws", "[exception]") {
    CATCH_CHECK_THROWS_MATCHES(THROW_RUNTIME_ERROR("this should throw!"),
                               mmotd::assertion::RuntimeError,
                               MmotdExceptionMatcher("this should throw!"));
}

CATCH_TEST_CASE("THROW_RANGE_ERROR throws", "[exception]") {
    CATCH_CHECK_THROWS_MATCHES(THROW_RANGE_ERROR("this should throw!"),
                               mmotd::assertion::RangeError,
                               MmotdExceptionMatcher("this should throw!"));
}

CATCH_TEST_CASE("THROW_OVERFLOW_ERROR throws", "[exception]") {
    CATCH_CHECK_THROWS_MATCHES(THROW_OVERFLOW_ERROR("this should throw!"),
                               mmotd::assertion::OverflowError,
                               MmotdExceptionMatcher("this should throw!"));
}

CATCH_TEST_CASE("THROW_UNDERFLOW_ERROR throws", "[exception]") {
    CATCH_CHECK_THROWS_MATCHES(THROW_UNDERFLOW_ERROR("this should throw!"),
                               mmotd::assertion::UnderflowError,
                               MmotdExceptionMatcher("this should throw!"));
}

} // namespace mmotd::test
