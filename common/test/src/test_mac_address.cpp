#include "common/include/mac_address.h"

#include <catch2/catch.hpp>

namespace mmotd::test {

TEST_CASE("MacAddress is zero constructed", "[MacAddress]") {
    auto mac_address = mmotd::networking::MacAddress{};
    CHECK(sizeof(mac_address) == 6);
}

TEST_CASE("MacAddress size is always constant", "[MacAddress]") {
    CHECK(sizeof(mmotd::networking::MacAddress) == 6);
}

} // namespace mmotd::test
