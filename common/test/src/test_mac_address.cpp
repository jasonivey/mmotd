#include "common/include/mac_address.h"

#include <catch2/catch.hpp>

namespace mmotd::test {

CATCH_TEST_CASE("MacAddress is zero constructed", "[MacAddress]") {
    auto mac_address = mmotd::networking::MacAddress{};
    CATCH_CHECK(sizeof(mac_address) == 6);
}

CATCH_TEST_CASE("MacAddress size is always constant", "[MacAddress]") {
    CATCH_CHECK(sizeof(mmotd::networking::MacAddress) == 6);
}

} // namespace mmotd::test
