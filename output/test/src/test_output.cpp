// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "output/include/output.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include <catch2/catch.hpp>

using Catch::Matchers::Equals;
using namespace std;

namespace mmotd::output::test {

CATCH_TEST_CASE("converting empty string returns empty string", "[OutputString]") {
    auto text = string{};
    auto converted_text = mmotd::output::FindAndReplaceColorSpecifications(text, true);
    CATCH_CHECK_THAT(text, Catch::Matchers::Equals(converted_text));
}

CATCH_TEST_CASE("converting string without color codes returns same string", "[OutputString]") {
    auto text = string{"simple string here"};
    auto converted_text = mmotd::output::FindAndReplaceColorSpecifications(text, true);
    CATCH_CHECK_THAT(text, Catch::Matchers::Equals(converted_text));
}

CATCH_TEST_CASE("convert invalid color codes", "[OutputString]") {
    CATCH_SECTION("convert color code with mising closing '%'") {
        auto src_text = string{"%color:bold_bright_greensimple string here"};
        auto converted_text = mmotd::output::FindAndReplaceColorSpecifications(src_text, true);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(src_text));
    }
    CATCH_SECTION("convert two color codes missing double '%%' in between") {
        auto src_text = string{"%color:bold_bright_green%color:bold_bright_red%simple string here"};
        auto converted_text = mmotd::output::FindAndReplaceColorSpecifications(src_text, true);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(src_text));
    }
}

CATCH_TEST_CASE("convert output string with single color code", "[OutputString]") {
    CATCH_SECTION("converting with single color code and no prefix") {
        auto src_text = string{"%color:purple%simple string here"};
        auto dst_text = string{"[color:purple]simple string here"};
        auto converted_text = mmotd::output::FindAndReplaceColorSpecifications(src_text, true);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
    CATCH_SECTION("converting with single color code and a prefix") {
        auto src_text = string{"simple prefix here %color:purple%simple string here"};
        auto dst_text = string{"simple prefix here [color:purple]simple string here"};
        auto converted_text = mmotd::output::FindAndReplaceColorSpecifications(src_text, true);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
    CATCH_SECTION("converting with single color code and string is a complete prefix") {
        auto src_text = string{"simple prefix here %color:purple%"};
        auto dst_text = string{"simple prefix here [color:purple]"};
        auto converted_text = mmotd::output::FindAndReplaceColorSpecifications(src_text, true);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
}

CATCH_TEST_CASE("convert output string with a reset code", "[OutputString]") {
    CATCH_SECTION("convert single reset code") {
        auto src_text = string{"%color:reset()%simple string here"};
        auto dst_text = string{"[color:reset()]simple string here"};
        auto converted_text = mmotd::output::FindAndReplaceColorSpecifications(src_text, true);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
}

CATCH_TEST_CASE("convert output string with two color codes", "[OutputString]") {
    auto src_text =
        string{"%color:hex(ffffff)%white text: %color:hex(AAAAAA)%the value which is another color%color:reset()%"};
    auto dst_text =
        string{"[color:hex(ffffff)]white text: [color:hex(AAAAAA)]the value which is another color[color:reset()]"};
    auto converted_text = mmotd::output::FindAndReplaceColorSpecifications(src_text, true);
    CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
}

CATCH_TEST_CASE("converting output string with color codes including resets", "[OutputString]") {
    CATCH_SECTION("start color codes with a reset") {
        auto src_text = string{
            "%color:reset():hex(ffffff)%white text: %color:hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:hex(ffffff)]white text: [color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = mmotd::output::FindAndReplaceColorSpecifications(src_text, true);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
    CATCH_SECTION("color codes which include multiple resets") {
        auto src_text = string{
            "%color:reset():hex(ffffff)%white text: %color:reset():hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:hex(ffffff)]white text: [color:reset()][color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = mmotd::output::FindAndReplaceColorSpecifications(src_text, true);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
}

CATCH_TEST_CASE("output string with color codes are found and replaced", "[OutputString]") {
    CATCH_SECTION("four color codes are converted") {
        auto src_text = string{
            "%color:reset()%%color:hex(ffffff)%white text: %color:hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:hex(ffffff)]white text: [color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = mmotd::output::FindAndReplaceColorSpecifications(src_text, true);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
    CATCH_SECTION("five color codes are converted") {
        auto src_text = string{
            "%color:reset()%%color:hex(ffffff)%white text: %color:reset()%%color:hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:hex(ffffff)]white text: [color:reset()][color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = mmotd::output::FindAndReplaceColorSpecifications(src_text, true);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
    CATCH_SECTION("eight color codes are converted") {
        auto src_text = string{
            "%color:reset()%%color:rgb(0,0,0)%%color:hex(ffffff)%white text: %color:reset()%%color:hex(AAAAAA)%the value which is another color%color:purple%%color:green%%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:rgb(0,0,0)][color:hex(ffffff)]white text: [color:reset()][color:hex(AAAAAA)]the value which is another color[color:purple][color:green][color:reset()]"};
        auto converted_text = mmotd::output::FindAndReplaceColorSpecifications(src_text, true);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
}

CATCH_TEST_CASE("ensure double delim is ignored when splitting", "[OutputString]") {
    auto src_text = string{"%color:reset():hex(ffffff)%system load: %color:hex(FF0000)%4.58%%color:reset()%"};
    auto dst_text = string{"[color:reset()][color:hex(ffffff)]system load: [color:hex(FF0000)]4.58%[color:reset()]"};
    auto converted_text = mmotd::output::FindAndReplaceColorSpecifications(src_text, true);
    CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
}

} // namespace mmotd::output::test
