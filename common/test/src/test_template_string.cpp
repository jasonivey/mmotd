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

TEST_CASE("color codes can be split", "[TemplateString]") {
    SECTION("splitting empty string gives empty container") {
        auto string_container = vector<string>{};
        auto color_definitions = TemplateStringTest::SplitColorStr(string{}, '%');
        CHECK_THAT(string_container, Catch::Matchers::Equals(color_definitions));
    }
    SECTION("splitting string{\"\"} gives empty container") {
        auto string_container = vector<string>{};
        auto color_definitions = TemplateStringTest::SplitColorStr("", '%');
        CHECK_THAT(string_container, Catch::Matchers::Equals(color_definitions));
    }
    SECTION("splitting string-no-delim gives container-one-element") {
        auto string_container = vector<string>{"asdf"};
        auto color_definitions = TemplateStringTest::SplitColorStr("asdf", '%');
        CHECK_THAT(string_container, Catch::Matchers::Equals(color_definitions));
    }
    SECTION("splitting string-dual-back-to-back-delim gives container-two-elements") {
        auto string_container = vector<string>{"asdf", "asdf"};
        auto color_definitions = TemplateStringTest::SplitColorStr("^asdf^^asdf^", '^');
        CHECK_THAT(string_container, Catch::Matchers::Equals(color_definitions));
    }
    SECTION("splitting string-dual-back-to-back-delim different delim gives container-two-elements") {
        auto string_container = vector<string>{"asdf", "asdf"};
        auto color_definitions = TemplateStringTest::SplitColorStr("%asdf%%asdf%", '%');
        CHECK_THAT(string_container, Catch::Matchers::Equals(color_definitions));
    }
    SECTION("splitting color-string-with-two-elements gives container-two-elements") {
        auto string_container = vector<string>{"color:reset():purple", "color:reset()"};
        auto color_definitions = TemplateStringTest::SplitColorStr("%color:reset():purple%%color:reset()%", '%');
        CHECK_THAT(string_container, Catch::Matchers::Equals(color_definitions));
    }
}

TEST_CASE("multiple color codes can be split", "[TemplateString]") {
    SECTION("splitting empty string gives empty container") {
        auto string_container = vector<string>{};
        auto color_definitions = TemplateStringTest::SplitMultipleColorStrs(vector<string>{}, '%');
        CHECK_THAT(string_container, Catch::Matchers::Equals(color_definitions));
    }
    SECTION("splitting multiple colors return multiple elements") {
        auto string_container = vector<string>{"color:asdf", "color:asdf", "color:asdf", "color:asdf"};
        auto color_definitions =
            TemplateStringTest::SplitMultipleColorStrs(vector<string>{"asdf:asdf:asdf", "asdf"}, ':');
        CHECK_THAT(string_container, Catch::Matchers::Equals(color_definitions));
    }
}

TEST_CASE("converting empty template string", "[TemplateString]") {
    SECTION("converting empty template string returns empty string") {
        auto text = string{};
        auto converted_text = TemplateStringTest::ConvertTemplateString(text);
        CHECK_THAT(text, Catch::Matchers::Equals(converted_text));
    }
}

TEST_CASE("converting template without any color codes returns string", "[TemplateString]") {
    SECTION("return string if no color codes found") {
        auto text = string{"simple string here"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(text);
        CHECK_THAT(text, Catch::Matchers::Equals(converted_text));
    }
}

TEST_CASE("convert template with single color code", "[TemplateString]") {
    SECTION("converting template with single color code") {
        auto src_text = string{"%color:purple%simple string here"};
        auto dst_text = string{"[color:purple]simple string here"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
}

TEST_CASE("convert template with a reset code", "[TemplateString]") {
    SECTION("convert single reset code") {
        auto src_text = string{"%color:reset()%simple string here"};
        auto dst_text = string{"[color:reset()]simple string here"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
}

TEST_CASE("convert template with two color codes", "[TemplateString]") {
    SECTION("converting empty template string returns empty string") {
        auto src_text =
            string{"%color:hex(ffffff)%white text: %color:hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text =
            string{"[color:hex(ffffff)]white text: [color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
}

TEST_CASE("converting color codes including resets", "[TemplateString]") {
    SECTION("start color codes with a reset") {
        auto src_text = string{
            "%color:reset():hex(ffffff)%white text: %color:hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:hex(ffffff)]white text: [color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
    SECTION("color codes which include multiple resets") {
        auto src_text = string{
            "%color:reset():hex(ffffff)%white text: %color:reset():hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:hex(ffffff)]white text: [color:reset()][color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
}

TEST_CASE("color codes are found and replaced", "[TemplateString]") {
    SECTION("four color codes are converted") {
        auto src_text = string{
            "%color:reset()%%color:hex(ffffff)%white text: %color:hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:hex(ffffff)]white text: [color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
    SECTION("five color codes are converted") {
        auto src_text = string{
            "%color:reset()%%color:hex(ffffff)%white text: %color:reset()%%color:hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:hex(ffffff)]white text: [color:reset()][color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
    SECTION("eight color codes are converted") {
        auto src_text = string{
            "%color:reset()%%color:rgb(0,0,0)%%color:hex(ffffff)%white text: %color:reset()%%color:hex(AAAAAA)%the value which is another color%color:purple%%color:green%%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:rgb(0,0,0)][color:hex(ffffff)]white text: [color:reset()][color:hex(AAAAAA)]the value which is another color[color:purple][color:green][color:reset()]"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
}

TEST_CASE("% character is ignored when splitting delim='%'", "[TemplateString]") {
    SECTION("ensure double delim=%% is ignored when splitting") {
        auto src_text = string{"%color:reset():hex(ffffff)%system load: %color:hex(FF0000)%4.58%%color:reset()%"};
        auto dst_text =
            string{"[color:reset()][color:hex(ffffff)]system load: [color:hex(FF0000)]4.58%[color:reset()]"};
        auto converted_text = TemplateStringTest::ConvertTemplateString(src_text);
        CHECK_THAT(converted_text, Catch::Matchers::Equals(dst_text));
    }
}

} // namespace mmotd::results::test
