// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/special_files.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <catch2/catch.hpp>

using namespace mmotd::core::special_files;
using namespace std;
using namespace std::literals;

namespace mmotd::results::test {

CATCH_TEST_CASE("empty GetEnvironmentValue", "[SpecialFiles]") {
    CATCH_CHECK(GetEnvironmentValue(string{}) == string{});
}

CATCH_TEST_CASE("undefined GetEnvironmentValue", "[SpecialFiles]") {
    CATCH_CHECK(GetEnvironmentValue("NO_WAY_THIS_ENV_VAR_IS_DEFINED"s) == string{});
}

CATCH_TEST_CASE("empty ExpandEnvironmentVariables", "[SpecialFiles]") {
    CATCH_CHECK(ExpandEnvironmentVariables(string{}) == string{});
}

CATCH_TEST_CASE("undefined ExpandEnvironmentVariables", "[SpecialFiles]") {
    CATCH_SECTION("dollar-curlybrace-variable-curlybrace") {
        auto expanded = ExpandEnvironmentVariables("this is the ${NO_WAY_THIS_ENV_VAR_IS_DEFINED} string"s);
        CATCH_CHECK(expanded == "this is the  string"s);
    }
    CATCH_SECTION("dollar-variable") {
        auto expanded = ExpandEnvironmentVariables("this is the $NO_WAY_THIS_ENV_VAR_IS_DEFINED string"s);
        CATCH_CHECK(expanded == "this is the  string"s);
    }
}

CATCH_TEST_CASE("first in string ExpandEnvironmentVariables", "[SpecialFiles]") {
    CATCH_SECTION("dollar-curlybrace-variable-curlybrace") {
        CATCH_CHECK(!ExpandEnvironmentVariables("${USER}"s).empty());
    }
    CATCH_SECTION("dollar-variable") { CATCH_CHECK(!ExpandEnvironmentVariables("$USER"s).empty()); }
}

CATCH_TEST_CASE("middle in string ExpandEnvironmentVariables", "[SpecialFiles]") {
    CATCH_SECTION("dollar-curlybrace-variable-curlybrace") {
        auto expanded = ExpandEnvironmentVariables("/filling/${USER}/filling/"s);
        auto trimmed = boost::replace_all_copy(expanded, "/filling/"s, ""s);
        CATCH_CHECK(!trimmed.empty());
    }
    CATCH_SECTION("dollar-variable") {
        auto expanded = ExpandEnvironmentVariables("/filling/$USER/filling/"s);
        auto trimmed = boost::replace_all_copy(expanded, "/filling/"s, ""s);
        CATCH_CHECK(!trimmed.empty());
    }
}

CATCH_TEST_CASE("last in string ExpandEnvironmentVariables", "[SpecialFiles]") {
    CATCH_SECTION("dollar-curlybrace-variable-curlybrace") {
        auto expanded = ExpandEnvironmentVariables("/filling/${HOME}"s);
        auto trimmed = expanded.substr("/filling/"s.size());
        CATCH_CHECK(!trimmed.empty());
    }
    CATCH_SECTION("dollar-variable") {
        auto expanded = ExpandEnvironmentVariables("/filling/$HOME"s);
        auto trimmed = expanded.substr("/filling/"s.size());
        CATCH_CHECK(!trimmed.empty());
    }
}

CATCH_TEST_CASE("multiple in string ExpandEnvironmentVariables", "[SpecialFiles]") {
    CATCH_SECTION("dollar-curlybrace-variable-curlybrace") {
        auto expanded = ExpandEnvironmentVariables("${USER}/filling/${HOME}"s);
        auto trimmed = boost::replace_all_copy(expanded, "/filling/"s, ""s);
        CATCH_CHECK(!trimmed.empty());
    }
    CATCH_SECTION("dollar-variable") {
        auto expanded = ExpandEnvironmentVariables("$USER/filling/$HOME"s);
        auto trimmed = boost::replace_all_copy(expanded, "/filling/"s, ""s);
        CATCH_CHECK(!trimmed.empty());
    }
}

CATCH_TEST_CASE("no variables ExpandEnvironmentVariables", "[SpecialFiles]") {
    auto expanded = ExpandEnvironmentVariables("/a/b/data/dir.txt"s);
    CATCH_CHECK(expanded == "/a/b/data/dir.txt"s);
}

} // namespace mmotd::results::test
