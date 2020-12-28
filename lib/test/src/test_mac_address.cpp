#include "lib/include/mac_address.h"

#include <gtest/gtest.h>

namespace mmotd {

class MacAddressTest : public ::testing::Test {
protected:
    MacAddressTest() {}

    ~MacAddressTest() override {}

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(MacAddressTest, IsZeroDefaultConstructable) {
    auto mac_address = MacAddress{};
    ASSERT_TRUE(sizeof(mac_address) == 6);
}

TEST_F(MacAddressTest, SizeIsAlwaysConstant) {
    static_assert(sizeof(MacAddress) == 6);
}

} // namespace mmotd
