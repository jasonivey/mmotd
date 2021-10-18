// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
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

CATCH_TEST_CASE("PositionIndex-constructor-takes-non-empty-container", "[PositionIndex]") {
    CATCH_CHECK_THROWS_MATCHES(PositionIndex(vector<int>{}, 0),
                               mmotd::assertion::Assertion,
                               MmotdExceptionMatcher("index must be less than the size of the container"));
}

CATCH_TEST_CASE("PositionIndex-entire-line", "[PositionIndex]") {
    auto position_index1 = PositionIndex(vector<int>{ENTIRE_LINE}, 0);
    CATCH_CHECK(position_index1.GetPosition() == PositionIndex::Position::FirstAndLast);
    CATCH_CHECK(position_index1.GetColumnNumber() == ENTIRE_LINE);
}

CATCH_TEST_CASE("PositionIndex-single-and-entire-line", "[PositionIndex]") {
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

CATCH_TEST_CASE("PositionIndex-single-element", "[PositionIndex]") {
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

CATCH_TEST_CASE("PositionIndex-two-elements", "[PositionIndex]") {
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

CATCH_TEST_CASE("PositionIndex-three-sequential-elements", "[PositionIndex]") {
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

CATCH_TEST_CASE("PositionIndex-three-elements", "[PositionIndex]") {
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

CATCH_TEST_CASE("PositionIndex-five-sequential-elements", "[PositionIndex]") {
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

CATCH_TEST_CASE("PositionIndex-five-elements", "[PositionIndex]") {
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

CATCH_TEST_CASE("PositionIndex-five-plus-elements", "[PositionIndex]") {
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

} // namespace mmotd::results::test
