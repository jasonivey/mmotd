// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/information.h"
#include "common/include/informations.h"
#include "common/results/include/template_column_items.h"
#include "common/results/include/template_string.h"
#include "common/results/include/template_substring.h"
#include "common/results/include/template_substring_range.h"
#include "common/results/include/template_substrings.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include <catch2/catch.hpp>

using Catch::Matchers::Equals;
using namespace std;

namespace mmotd::results::test {

class TemplateStringTest {
public:
    static string ConvertTemplateString(string text) {
        auto template_substrings = TemplateString::GenerateTemplateSubstrings(text);
        return template_substrings.to_string();
    }

    static vector<string> SplitColorStr(const string &text, const char delim) {
        return TemplateString::SplitColorCodeDefinitions(text, delim);
    }

    static vector<string> SplitMultipleColorStrs(const vector<string> &color_strs, const char delimeter) {
        return TemplateString::SplitMultipleColorCodeDefinitions(color_strs, delimeter);
    }
};

CATCH_TEST_CASE("color codes can be split", "[TemplateString]") {
    CATCH_SECTION("splitting empty string gives empty container") {
        auto string_container = vector<string>{};
        auto color_definitions = TemplateStringTest::SplitColorStr(string{}, '%');
        CATCH_CHECK_THAT(string_container, Catch::Matchers::Equals(color_definitions));
    }
    CATCH_SECTION("splitting string{\"\"} gives empty container") {
        auto string_container = vector<string>{};
        auto color_definitions = TemplateStringTest::SplitColorStr("", '%');
        CATCH_CHECK_THAT(string_container, Catch::Matchers::Equals(color_definitions));
    }
    CATCH_SECTION("splitting string-no-delim gives container-one-element") {
        auto string_container = vector<string>{"asdf"};
        auto color_definitions = TemplateStringTest::SplitColorStr("asdf", '%');
        CATCH_CHECK_THAT(string_container, Catch::Matchers::Equals(color_definitions));
    }
    CATCH_SECTION("splitting string-dual-back-to-back-delim gives container-two-elements") {
        auto string_container = vector<string>{"asdf", "asdf"};
        auto color_definitions = TemplateStringTest::SplitColorStr("^asdf^^asdf^", '^');
        CATCH_CHECK_THAT(string_container, Catch::Matchers::Equals(color_definitions));
    }
    CATCH_SECTION("splitting string-dual-back-to-back-delim different delim gives container-two-elements") {
        auto string_container = vector<string>{"asdf", "asdf"};
        auto color_definitions = TemplateStringTest::SplitColorStr("%asdf%%asdf%", '%');
        CATCH_CHECK_THAT(string_container, Catch::Matchers::Equals(color_definitions));
    }
    CATCH_SECTION("splitting color-string-with-two-elements gives container-two-elements") {
        auto string_container = vector<string>{"color:reset():purple", "color:reset()"};
        auto color_definitions = TemplateStringTest::SplitColorStr("%color:reset():purple%%color:reset()%", '%');
        CATCH_CHECK_THAT(string_container, Catch::Matchers::Equals(color_definitions));
    }
}

CATCH_TEST_CASE("multiple color codes can be split", "[TemplateString]") {
    CATCH_SECTION("splitting empty string gives empty container") {
        auto string_container = vector<string>{};
        auto color_definitions = TemplateStringTest::SplitMultipleColorStrs(vector<string>{}, '%');
        CATCH_CHECK_THAT(string_container, Catch::Matchers::Equals(color_definitions));
    }
    CATCH_SECTION("splitting multiple colors return multiple elements") {
        auto string_container = vector<string>{"color:asdf", "color:asdf", "color:asdf", "color:asdf"};
        auto color_definitions =
            TemplateStringTest::SplitMultipleColorStrs(vector<string>{"asdf:asdf:asdf", "asdf"}, ':');
        CATCH_CHECK_THAT(string_container, Catch::Matchers::Equals(color_definitions));
    }
}

CATCH_TEST_CASE("converting empty template string", "[TemplateString]") {
    CATCH_SECTION("converting empty template string returns empty string") {
        auto text = string{};
        auto converted_text = TemplateStringTest::ConvertTemplateString(text);
        CATCH_CHECK_THAT(text, Catch::Matchers::Equals(converted_text));
    }
}

CATCH_TEST_CASE("converting template without any color codes returns string", "[TemplateString]") {
    CATCH_SECTION("return string if no color codes found") {
        auto text = string{"simple string here"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(text);
        CATCH_CHECK_THAT(text, Catch::Matchers::Equals(converted_text));
    }
}

CATCH_TEST_CASE("convert template with single color code", "[TemplateString]") {
    CATCH_SECTION("converting template with single color code") {
        auto src_text = string{"%color:purple%simple string here"};
        auto dst_text = string{"[color:purple]simple string here"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
}

CATCH_TEST_CASE("convert template with a reset code", "[TemplateString]") {
    CATCH_SECTION("convert single reset code") {
        auto src_text = string{"%color:reset()%simple string here"};
        auto dst_text = string{"[color:reset()]simple string here"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
}

CATCH_TEST_CASE("convert template with two color codes", "[TemplateString]") {
    CATCH_SECTION("converting empty template string returns empty string") {
        auto src_text =
            string{"%color:hex(ffffff)%white text: %color:hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text =
            string{"[color:hex(ffffff)]white text: [color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
}

CATCH_TEST_CASE("converting color codes including resets", "[TemplateString]") {
    CATCH_SECTION("start color codes with a reset") {
        auto src_text = string{
            "%color:reset():hex(ffffff)%white text: %color:hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:hex(ffffff)]white text: [color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
    CATCH_SECTION("color codes which include multiple resets") {
        auto src_text = string{
            "%color:reset():hex(ffffff)%white text: %color:reset():hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:hex(ffffff)]white text: [color:reset()][color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
}

CATCH_TEST_CASE("color codes are found and replaced", "[TemplateString]") {
    CATCH_SECTION("four color codes are converted") {
        auto src_text = string{
            "%color:reset()%%color:hex(ffffff)%white text: %color:hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:hex(ffffff)]white text: [color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
    CATCH_SECTION("five color codes are converted") {
        auto src_text = string{
            "%color:reset()%%color:hex(ffffff)%white text: %color:reset()%%color:hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:hex(ffffff)]white text: [color:reset()][color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
    CATCH_SECTION("eight color codes are converted") {
        auto src_text = string{
            "%color:reset()%%color:rgb(0,0,0)%%color:hex(ffffff)%white text: %color:reset()%%color:hex(AAAAAA)%the value which is another color%color:purple%%color:green%%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:rgb(0,0,0)][color:hex(ffffff)]white text: [color:reset()][color:hex(AAAAAA)]the value which is another color[color:purple][color:green][color:reset()]"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
}

CATCH_TEST_CASE("% character is ignored when splitting delim='%'", "[TemplateString]") {
    CATCH_SECTION("ensure double delim=%% is ignored when splitting") {
        auto src_text = string{"%color:reset():hex(ffffff)%system load: %color:hex(FF0000)%4.58%%color:reset()%"};
        auto dst_text =
            string{"[color:reset()][color:hex(ffffff)]system load: [color:hex(FF0000)]4.58%[color:reset()]"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CATCH_CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
}

} // namespace mmotd::results::test
