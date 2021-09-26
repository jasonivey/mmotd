#include "common/assertion/include/assertion.h"
#include "common/results/include/output_position_index.h"
#include "common/results/include/template_column_items.h"
#include "common/test/include/exception_matcher.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include <catch2/catch.hpp>

using Catch::Matchers::Equals;
using mmotd::results::PositionIndex;
using mmotd::results::data::ENTIRE_LINE;
using mmotd::test::MmotdExceptionMatcher;
using namespace std;

namespace mmotd::results::test {

CATCH_TEST_CASE("constructor", "[PositionIndex]") {
    CATCH_SECTION("constructor does not take empty container") {
        CATCH_CHECK_THROWS_MATCHES(PositionIndex(vector<int>{}, 0),
                                   mmotd::assertion::Assertion,
                                   MmotdExceptionMatcher("index must be less than the size of the container"));
    }

    CATCH_SECTION("ENTIRE_LINE matches FirstAndLast") {
        auto position_index1 = PositionIndex(vector<int>{ENTIRE_LINE}, 0);
        CATCH_CHECK(position_index1.GetPosition() == PositionIndex::Position::FirstAndLast);
        CATCH_CHECK(position_index1.GetColumnNumber() == ENTIRE_LINE);
    }

    CATCH_SECTION("ENTIRE_LINE and 1 other column matches FirstAndLast and FirstAndLast") {
        auto position_index1a = PositionIndex(vector<int>{ENTIRE_LINE, 0}, 0);
        auto position_index1b = PositionIndex(vector<int>{ENTIRE_LINE, 0}, 1);
        CATCH_CHECK(position_index1a.GetPosition() == PositionIndex::Position::FirstAndLast);
        CATCH_CHECK(position_index1a.GetColumnNumber() == ENTIRE_LINE);
        CATCH_CHECK(position_index1b.GetPosition() == PositionIndex::Position::FirstAndLast);
        CATCH_CHECK(position_index1b.GetColumnNumber() == 0);

        auto position_index2a = PositionIndex(vector<int>{ENTIRE_LINE, 666}, 0);
        auto position_index2b = PositionIndex(vector<int>{ENTIRE_LINE, 666}, 1);
        CATCH_CHECK(position_index2a.GetPosition() == PositionIndex::Position::FirstAndLast);
        CATCH_CHECK(position_index2a.GetColumnNumber() == ENTIRE_LINE);
        CATCH_CHECK(position_index2b.GetPosition() == PositionIndex::Position::FirstAndLast);
        CATCH_CHECK(position_index2b.GetColumnNumber() == 666);
    }

    CATCH_SECTION("single element matches FirstAndLast") {
        auto position_index1 = PositionIndex(vector<int>{0}, 0);
        CATCH_CHECK(position_index1.GetPosition() == PositionIndex::Position::FirstAndLast);
        CATCH_CHECK(position_index1.GetColumnNumber() == 0);

        auto position_index2 = PositionIndex(vector<int>{22}, 0);
        CATCH_CHECK(position_index2.GetPosition() == PositionIndex::Position::FirstAndLast);
        CATCH_CHECK(position_index2.GetColumnNumber() == 22);

        auto position_index3 = PositionIndex(vector<int>{3000}, 0);
        CATCH_CHECK(position_index3.GetPosition() == PositionIndex::Position::FirstAndLast);
        CATCH_CHECK(position_index3.GetColumnNumber() == 3000);
    }

    CATCH_SECTION("two elements matches First and Last") {
        auto position_index1a = PositionIndex(vector<int>{0, 1}, 0);
        auto position_index1b = PositionIndex(vector<int>{0, 1}, 1);
        CATCH_CHECK(position_index1a.GetPosition() == PositionIndex::Position::First);
        CATCH_CHECK(position_index1a.GetColumnNumber() == 0);
        CATCH_CHECK(position_index1b.GetPosition() == PositionIndex::Position::Last);
        CATCH_CHECK(position_index1b.GetColumnNumber() == 1);

        auto position_index2a = PositionIndex(vector<int>{22, 23}, 0);
        auto position_index2b = PositionIndex(vector<int>{22, 23}, 1);
        CATCH_CHECK(position_index2a.GetPosition() == PositionIndex::Position::First);
        CATCH_CHECK(position_index2a.GetColumnNumber() == 22);
        CATCH_CHECK(position_index2b.GetPosition() == PositionIndex::Position::Last);
        CATCH_CHECK(position_index2b.GetColumnNumber() == 23);

        auto position_index3a = PositionIndex(vector<int>{3000, 4000}, 0);
        auto position_index3b = PositionIndex(vector<int>{3000, 4000}, 1);
        CATCH_CHECK(position_index3a.GetPosition() == PositionIndex::Position::First);
        CATCH_CHECK(position_index3a.GetColumnNumber() == 3000);
        CATCH_CHECK(position_index3b.GetPosition() == PositionIndex::Position::Last);
        CATCH_CHECK(position_index3b.GetColumnNumber() == 4000);
    }

    CATCH_SECTION("three elements matches First, Middle and Last") {
        auto container1 = vector<int>{0, 1, 2};
        auto position_index0 = PositionIndex(container1, 0);
        auto position_index1 = PositionIndex(container1, 1);
        auto position_index2 = PositionIndex(container1, 2);
        CATCH_CHECK(position_index0.GetPosition() == PositionIndex::Position::First);
        CATCH_CHECK(position_index0.GetColumnNumber() == 0);
        CATCH_CHECK(position_index1.GetPosition() == PositionIndex::Position::Middle);
        CATCH_CHECK(position_index1.GetColumnNumber() == 1);
        CATCH_CHECK(position_index2.GetPosition() == PositionIndex::Position::Last);
        CATCH_CHECK(position_index2.GetColumnNumber() == 2);
    }

    CATCH_SECTION("three scattered elements matches First, Middle and Last") {
        auto container1 = vector<int>{112, 151, 188};
        auto position_index0 = PositionIndex(container1, 0);
        auto position_index1 = PositionIndex(container1, 1);
        auto position_index2 = PositionIndex(container1, 2);
        CATCH_CHECK(position_index0.GetPosition() == PositionIndex::Position::First);
        CATCH_CHECK(position_index0.GetColumnNumber() == 112);
        CATCH_CHECK(position_index1.GetPosition() == PositionIndex::Position::Middle);
        CATCH_CHECK(position_index1.GetColumnNumber() == 151);
        CATCH_CHECK(position_index2.GetPosition() == PositionIndex::Position::Last);
        CATCH_CHECK(position_index2.GetColumnNumber() == 188);
    }

    CATCH_SECTION("five elements matches First, Middle and Last") {
        auto container1 = vector<int>{0, 1, 2, 3, 4};
        auto position_index0 = PositionIndex(container1, 0);
        auto position_index1 = PositionIndex(container1, 1);
        auto position_index2 = PositionIndex(container1, 2);
        auto position_index3 = PositionIndex(container1, 3);
        auto position_index4 = PositionIndex(container1, 4);
        CATCH_CHECK(position_index0.GetPosition() == PositionIndex::Position::First);
        CATCH_CHECK(position_index0.GetColumnNumber() == 0);
        CATCH_CHECK(position_index1.GetPosition() == PositionIndex::Position::Middle);
        CATCH_CHECK(position_index1.GetColumnNumber() == 1);
        CATCH_CHECK(position_index2.GetPosition() == PositionIndex::Position::Middle);
        CATCH_CHECK(position_index2.GetColumnNumber() == 2);
        CATCH_CHECK(position_index3.GetPosition() == PositionIndex::Position::Middle);
        CATCH_CHECK(position_index3.GetColumnNumber() == 3);
        CATCH_CHECK(position_index4.GetPosition() == PositionIndex::Position::Last);
        CATCH_CHECK(position_index4.GetColumnNumber() == 4);
    }

    CATCH_SECTION("five scattered elements matches First, Middle and Last") {
        auto container1 = vector<int>{33, 44, 55, 66, 77};
        auto position_index0 = PositionIndex(container1, 0);
        auto position_index1 = PositionIndex(container1, 1);
        auto position_index2 = PositionIndex(container1, 2);
        auto position_index3 = PositionIndex(container1, 3);
        auto position_index4 = PositionIndex(container1, 4);
        CATCH_CHECK(position_index0.GetPosition() == PositionIndex::Position::First);
        CATCH_CHECK(position_index0.GetColumnNumber() == 33);
        CATCH_CHECK(position_index1.GetPosition() == PositionIndex::Position::Middle);
        CATCH_CHECK(position_index1.GetColumnNumber() == 44);
        CATCH_CHECK(position_index2.GetPosition() == PositionIndex::Position::Middle);
        CATCH_CHECK(position_index2.GetColumnNumber() == 55);
        CATCH_CHECK(position_index3.GetPosition() == PositionIndex::Position::Middle);
        CATCH_CHECK(position_index3.GetColumnNumber() == 66);
        CATCH_CHECK(position_index4.GetPosition() == PositionIndex::Position::Last);
        CATCH_CHECK(position_index4.GetColumnNumber() == 77);
    }

    CATCH_SECTION("five+ scattered elements with ENTIRE_LINE matches First, Middle and Last") {
        auto container1 = vector<int>{33, 44, ENTIRE_LINE, 55, 66, 77};
        auto position_index0 = PositionIndex(container1, 0);
        auto position_index1 = PositionIndex(container1, 1);
        auto position_index2 = PositionIndex(container1, 2);
        auto position_index3 = PositionIndex(container1, 3);
        auto position_index4 = PositionIndex(container1, 4);
        auto position_index5 = PositionIndex(container1, 5);
        CATCH_CHECK(position_index0.GetPosition() == PositionIndex::Position::First);
        CATCH_CHECK(position_index0.GetColumnNumber() == 33);
        CATCH_CHECK(position_index1.GetPosition() == PositionIndex::Position::Middle);
        CATCH_CHECK(position_index1.GetColumnNumber() == 44);
        CATCH_CHECK(position_index2.GetPosition() == PositionIndex::Position::FirstAndLast);
        CATCH_CHECK(position_index2.GetColumnNumber() == ENTIRE_LINE);
        CATCH_CHECK(position_index3.GetPosition() == PositionIndex::Position::Middle);
        CATCH_CHECK(position_index3.GetColumnNumber() == 55);
        CATCH_CHECK(position_index4.GetPosition() == PositionIndex::Position::Middle);
        CATCH_CHECK(position_index4.GetColumnNumber() == 66);
        CATCH_CHECK(position_index5.GetPosition() == PositionIndex::Position::Last);
        CATCH_CHECK(position_index5.GetColumnNumber() == 77);
    }
}

/*
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
*/

} // namespace mmotd::results::test
