#include "common/include/information.h"
#include "common/include/informations.h"
#include "common/include/tty_template_data.h"
#include "common/include/tty_template_string.h"
#include "common/include/tty_template_substring.h"
#include "common/include/tty_template_substring_range.h"
#include "common/include/tty_template_substrings.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include <gtest/gtest.h>

using namespace std;

namespace {

bool StringsAreEqual(const string &a, const string &b) {
    return a == b;
}

bool ContainersAreEqual(const vector<string> &a, const vector<string> &b) {
    return std::equal(std::begin(a), std::end(a), std::begin(b), std::end(b));
}

} // namespace

namespace mmotd::tty_template::tty_string::test {

class TemplateStringTest : public ::testing::Test {
protected:
    TemplateStringTest() {}

    ~TemplateStringTest() override {}

    void SetUp() override {}

    void TearDown() override {}

    string ConvertTemplateString(string text);
    vector<string> SplitColorStr(const string &text, const char delim);
    vector<string> SplitMultipleColorStrs(const vector<string> &color_strs, const char delimeter);

    mmotd::information::Informations informations_;
    mmotd::tty_template::data::TemplateColumnItem item_;
};

string TemplateStringTest::ConvertTemplateString(string text) {
    auto template_string = mmotd::tty_template::tty_string::TemplateString{text};
    auto template_substrings = TemplateString::GenerateTemplateSubstrings(text);
    return template_substrings.to_string();
}

vector<string> TemplateStringTest::SplitColorStr(const string &text, const char delim) {
    return TemplateString::SplitColorCodeDefinitions(text, delim);
}

vector<string> TemplateStringTest::SplitMultipleColorStrs(const vector<string> &color_strs, const char delimeter) {
    return TemplateString::SplitMultipleColorCodeDefinitions(color_strs, delimeter);
}

TEST_F(TemplateStringTest, TestSplitColorCodeDefinitions) {
    {
        auto string_container = vector<string>{};
        auto color_definitions = SplitColorStr(string{}, '%');
        EXPECT_PRED2(ContainersAreEqual, string_container, color_definitions);
    }
    {
        auto string_container = vector<string>{};
        auto color_definitions = SplitColorStr("", '%');
        EXPECT_PRED2(ContainersAreEqual, string_container, color_definitions);
    }
    {
        auto string_container = vector<string>{"asdf"};
        auto color_definitions = SplitColorStr("asdf", '%');
        EXPECT_PRED2(ContainersAreEqual, string_container, color_definitions);
    }
    {
        auto string_container = vector<string>{"asdf", "asdf"};
        auto color_definitions = SplitColorStr("^asdf^^asdf^", '^');
        EXPECT_PRED2(ContainersAreEqual, string_container, color_definitions);
    }
    {
        auto string_container = vector<string>{"asdf", "asdf"};
        auto color_definitions = SplitColorStr("%asdf%%asdf%", '%');
        EXPECT_PRED2(ContainersAreEqual, string_container, color_definitions);
    }
    {
        auto string_container = vector<string>{"color:reset():purple", "color:reset()"};
        auto color_definitions = SplitColorStr("%color:reset():purple%%color:reset()%", '%');
        EXPECT_PRED2(ContainersAreEqual, string_container, color_definitions);
    }
}

TEST_F(TemplateStringTest, TestSplitMultipleColorStrs) {
    {
        auto string_container = vector<string>{};
        auto color_definitions = SplitMultipleColorStrs(vector<string>{}, '%');
        EXPECT_PRED2(ContainersAreEqual, string_container, color_definitions);
    }
    {
        // auto string_container = vector<string>{""};
        // auto color_definitions = SplitMultipleColorStrs(vector<string>{""}, '%');
        // EXPECT_PRED2(ContainersAreEqual, string_container, color_definitions);
    } {
        auto string_container = vector<string>{"color:asdf", "color:asdf", "color:asdf", "color:asdf"};
        auto color_definitions = SplitMultipleColorStrs(vector<string>{"asdf:asdf:asdf", "asdf"}, ':');
        EXPECT_PRED2(ContainersAreEqual, string_container, color_definitions);
    }
#if 0
    {
        auto string_container = vector<string>{"asdf", "asdf"};
        auto color_definitions = SplitMultipleColorStrs("^asdf^^asdf^", '^');
        EXPECT_PRED2(ContainersAreEqual, string_container, color_definitions);
    }
    {
        auto string_container = vector<string>{"asdf", "asdf"};
        auto color_definitions = SplitMultipleColorStrs("%asdf%%asdf%", '%');
        EXPECT_PRED2(ContainersAreEqual, string_container, color_definitions);
    }
    {
        auto string_container = vector<string>{"color:reset():purple", "color:reset()"};
        auto color_definitions = SplitMultipleColorStrs("%color:reset():purple%%color:reset()%", '%');
        EXPECT_PRED2(ContainersAreEqual, string_container, color_definitions);
    }
#endif
}

TEST_F(TemplateStringTest, EmptyString) {
    auto text = string{};
    auto converted_text = ConvertTemplateString(text);

    EXPECT_PRED2(StringsAreEqual, converted_text, text);
}

TEST_F(TemplateStringTest, TestSimpleString) {
    auto text = string{"simple string here"};
    auto converted_text = ConvertTemplateString(text);

    EXPECT_PRED2(StringsAreEqual, converted_text, text);
}

TEST_F(TemplateStringTest, TestSingleTemplateParameter) {
    auto src_text = string{"%color:purple%simple string here"};
    auto dst_text = string{"[color:purple]simple string here"};
    auto converted_text = ConvertTemplateString(src_text);

    EXPECT_PRED2(StringsAreEqual, converted_text, dst_text);
}

TEST_F(TemplateStringTest, TestSingleResetTemplateParameter) {
    auto src_text = string{"%color:reset()%simple string here"};
    auto dst_text = string{"[color:reset()]simple string here"};
    auto converted_text = ConvertTemplateString(src_text);

    EXPECT_PRED2(StringsAreEqual, converted_text, dst_text);
}

TEST_F(TemplateStringTest, TestDoubleColorTemplateParameter) {
    auto src_text =
        string{"%color:hex(ffffff)%white text: %color:hex(AAAAAA)%the value which is another color%color:reset()%"};
    auto dst_text =
        string{"[color:hex(ffffff)]white text: [color:hex(AAAAAA)]the value which is another color[color:reset()]"};
    auto converted_text = ConvertTemplateString(src_text);

    EXPECT_PRED2(StringsAreEqual, converted_text, dst_text);
}

TEST_F(TemplateStringTest, TestDoubledColorTemplateParameter) {
    {
        auto src_text = string{
            "%color:reset():hex(ffffff)%white text: %color:hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:hex(ffffff)]white text: [color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = ConvertTemplateString(src_text);

        EXPECT_PRED2(StringsAreEqual, converted_text, dst_text);
    }
    {
        auto src_text = string{
            "%color:reset():hex(ffffff)%white text: %color:reset():hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:hex(ffffff)]white text: [color:reset()][color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = ConvertTemplateString(src_text);

        EXPECT_PRED2(StringsAreEqual, converted_text, dst_text);
    }
}

TEST_F(TemplateStringTest, TestRepeatedSplitColorTemplateParameter) {
    {
        auto src_text = string{
            "%color:reset()%%color:hex(ffffff)%white text: %color:hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:hex(ffffff)]white text: [color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = ConvertTemplateString(src_text);

        EXPECT_PRED2(StringsAreEqual, converted_text, dst_text);
    }
    {
        auto src_text = string{
            "%color:reset()%%color:hex(ffffff)%white text: %color:reset()%%color:hex(AAAAAA)%the value which is another color%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:hex(ffffff)]white text: [color:reset()][color:hex(AAAAAA)]the value which is another color[color:reset()]"};
        auto converted_text = ConvertTemplateString(src_text);

        EXPECT_PRED2(StringsAreEqual, converted_text, dst_text);
    }
    {
        auto src_text = string{
            "%color:reset()%%color:rgb(0,0,0)%%color:hex(ffffff)%white text: %color:reset()%%color:hex(AAAAAA)%the value which is another color%color:purple%%color:green%%color:reset()%"};
        auto dst_text = string{
            "[color:reset()][color:rgb(0,0,0)][color:hex(ffffff)]white text: [color:reset()][color:hex(AAAAAA)]the value which is another color[color:purple][color:green][color:reset()]"};
        auto converted_text = ConvertTemplateString(src_text);

        EXPECT_PRED2(StringsAreEqual, converted_text, dst_text);
    }
}

TEST_F(TemplateStringTest, TestEmbeddedPercentCharacter) {
    auto src_text = string{"%color:reset():hex(ffffff)%system load: %color:hex(FF0000)%4.58%%color:reset()%"};
    auto dst_text = string{"[color:reset()][color:hex(ffffff)]system load: [color:hex(FF0000)]4.58%[color:reset()]"};
    auto converted_text = ConvertTemplateString(src_text);

    EXPECT_PRED2(StringsAreEqual, converted_text, dst_text);
}

#if 0


(color:(?!reset)[^%]+)

(%(color(?::(reset\(\)))?(?::([^%]+))?)%)+

simple string here

%color:purple%simple string here

%color:purple%%color:green%simple string here

%color:reset()%simple string here

%color:hex(32CD32)%Log in: %color:reset()%%color:hex(FFFFFF)%%ID_LAST_LOGIN_LOGIN_TIME%

%color:hex(32CD32)%Log out: %color:reset()%%color:hex(FFFFFF)%%ID_LAST_LOGIN_LOGOUT_TIME%

%color:reset():purple%simple string here

%color:reset()%simple string here

%color:reset():hex(32CD32)%Log in: %color:reset():hex(FFFFFF)%ID_LAST_LOGIN_LOGIN_TIME%

%color:reset():hex(32CD32)%Log out: %color:reset():hex(FFFFFF)%%ID_LAST_LOGIN_LOGOUT_TIME%

#endif

#if 0

(?:%color[^%]+%)+

simple string here

%color:purple%simple string here

%color:purple%%color:green%simple string here

%color:reset()%simple string here

%color:hex(32CD32)%Log in: %color:reset()%%color:hex(FFFFFF)%%ID_LAST_LOGIN_LOGIN_TIME%

%color:hex(32CD32)%Log out: %color:reset()%%color:hex(FFFFFF)%%ID_LAST_LOGIN_LOGOUT_TIME%

%color:reset():purple%simple string here

%color:reset()%simple string here

%color:reset():hex(32CD32)%Log in: %color:reset():hex(FFFFFF)%ID_LAST_LOGIN_LOGIN_TIME%

%color:reset():hex(32CD32)%Log out: %color:reset():hex(FFFFFF)%%ID_LAST_LOGIN_LOGOUT_TIME%
#endif

} // namespace mmotd::tty_template::tty_string::test
