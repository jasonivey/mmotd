#include "common/include/mac_address.h"

#include <gtest/gtest.h>

namespace mmotd::test {

class MacAddressTest : public ::testing::Test {
protected:
    MacAddressTest() {}

    ~MacAddressTest() override {}

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(MacAddressTest, IsZeroDefaultConstructable) {
    auto mac_address = mmotd::networking::MacAddress{};
    ASSERT_TRUE(sizeof(mac_address) == 6);
}

TEST_F(MacAddressTest, SizeIsAlwaysConstant) {
    static_assert(sizeof(mmotd::networking::MacAddress) == 6);
}

} // namespace mmotd::test
