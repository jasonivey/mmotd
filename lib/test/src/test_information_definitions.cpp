#include "common/include/information_definitions.h"

#include <gtest/gtest.h>

namespace mmotd::test {

class InformationDefinitionsTest : public ::testing::Test {
protected:
    InformationDefinitionsTest() {}

    ~InformationDefinitionsTest() override {}

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(InformationDefinitionsTest, IsCategoryIdAccurate) {
    using namespace mmotd::information;

    ASSERT_EQ(CategoryId::ID_GENERAL, static_cast<CategoryId>(MakeCategoryId(110)));
    ASSERT_EQ(CategoryId::ID_NETWORK_INFO, static_cast<CategoryId>(MakeCategoryId(111)));
    ASSERT_EQ(CategoryId::ID_BOOT_TIME, static_cast<CategoryId>(MakeCategoryId(112)));
    ASSERT_EQ(CategoryId::ID_EXTERNAL_NETWORK_INFO, static_cast<CategoryId>(MakeCategoryId(113)));
    ASSERT_EQ(CategoryId::ID_LOCATION_INFO, static_cast<CategoryId>(MakeCategoryId(114)));
    ASSERT_EQ(CategoryId::ID_FILE_SYSTEM, static_cast<CategoryId>(MakeCategoryId(115)));
    ASSERT_EQ(CategoryId::ID_FORTUNE, static_cast<CategoryId>(MakeCategoryId(116)));
    ASSERT_EQ(CategoryId::ID_LAST_LOGIN, static_cast<CategoryId>(MakeCategoryId(117)));
    ASSERT_EQ(CategoryId::ID_PROCESSOR_COUNT, static_cast<CategoryId>(MakeCategoryId(118)));
    ASSERT_EQ(CategoryId::ID_LOAD_AVERAGE, static_cast<CategoryId>(MakeCategoryId(119)));
    ASSERT_EQ(CategoryId::ID_MEMORY_USAGE, static_cast<CategoryId>(MakeCategoryId(120)));
    ASSERT_EQ(CategoryId::ID_PROCESSES, static_cast<CategoryId>(MakeCategoryId(121)));
    ASSERT_EQ(CategoryId::ID_SWAP_USAGE, static_cast<CategoryId>(MakeCategoryId(122)));
    ASSERT_EQ(CategoryId::ID_SYSTEM_INFORMATION, static_cast<CategoryId>(MakeCategoryId(123)));
    ASSERT_EQ(CategoryId::ID_LOGGED_IN, static_cast<CategoryId>(MakeCategoryId(124)));
    ASSERT_EQ(CategoryId::ID_WEATHER, static_cast<CategoryId>(MakeCategoryId(125)));

    ASSERT_EQ(CategoryId::ID_GENERAL, static_cast<CategoryId>(0x6Ellu << 32));
    ASSERT_EQ(CategoryId::ID_NETWORK_INFO, static_cast<CategoryId>(0x6Fllu << 32));
    ASSERT_EQ(CategoryId::ID_BOOT_TIME, static_cast<CategoryId>(0x70llu << 32));
    ASSERT_EQ(CategoryId::ID_EXTERNAL_NETWORK_INFO, static_cast<CategoryId>(0x71llu << 32));
    ASSERT_EQ(CategoryId::ID_LOCATION_INFO, static_cast<CategoryId>(0x72llu << 32));
    ASSERT_EQ(CategoryId::ID_FILE_SYSTEM, static_cast<CategoryId>(0x73llu << 32));
    ASSERT_EQ(CategoryId::ID_FORTUNE, static_cast<CategoryId>(0x74llu << 32));
    ASSERT_EQ(CategoryId::ID_LAST_LOGIN, static_cast<CategoryId>(0x75llu << 32));
    ASSERT_EQ(CategoryId::ID_PROCESSOR_COUNT, static_cast<CategoryId>(0x76llu << 32));
    ASSERT_EQ(CategoryId::ID_LOAD_AVERAGE, static_cast<CategoryId>(0x77llu << 32));
    ASSERT_EQ(CategoryId::ID_MEMORY_USAGE, static_cast<CategoryId>(0x78llu << 32));
    ASSERT_EQ(CategoryId::ID_PROCESSES, static_cast<CategoryId>(0x79llu << 32));
    ASSERT_EQ(CategoryId::ID_SWAP_USAGE, static_cast<CategoryId>(0x7Allu << 32));
    ASSERT_EQ(CategoryId::ID_SYSTEM_INFORMATION, static_cast<CategoryId>(0x7Bllu << 32));
    ASSERT_EQ(CategoryId::ID_LOGGED_IN, static_cast<CategoryId>(0x7Cllu << 32));
    ASSERT_EQ(CategoryId::ID_WEATHER, static_cast<CategoryId>(0x7Dllu << 32));
}

TEST_F(InformationDefinitionsTest, IsInformationIdAccurate) {
    using namespace mmotd::information;

    ASSERT_EQ(InformationId::ID_GENERAL_GREETING,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_GENERAL, 901)));
    ASSERT_EQ(InformationId::ID_GENERAL_USER_NAME,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_GENERAL, 902)));
    ASSERT_EQ(InformationId::ID_GENERAL_LOCAL_DATE_TIME,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_GENERAL, 903)));
    ASSERT_EQ(InformationId::ID_GENERAL_LOCAL_TIME_EMOJI,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_GENERAL, 904)));

    ASSERT_EQ(InformationId::ID_NETWORK_INFO_IP,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_NETWORK_INFO, 1001)));
    ASSERT_EQ(InformationId::ID_NETWORK_INFO_MAC,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_NETWORK_INFO, 1002)));
    ASSERT_EQ(InformationId::ID_NETWORK_INFO_INTERFACE_NAME,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_NETWORK_INFO, 1003)));

    ASSERT_EQ(InformationId::ID_BOOT_TIME_BOOT_TIME,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_BOOT_TIME, 2001)));

    ASSERT_EQ(InformationId::ID_EXTERNAL_NETWORK_INFO_EXTERNAL_IP,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_EXTERNAL_NETWORK_INFO, 3001)));

    ASSERT_EQ(InformationId::ID_LOCATION_INFO_CITY,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_LOCATION_INFO, 4001)));
    ASSERT_EQ(InformationId::ID_LOCATION_INFO_COUNTRY,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_LOCATION_INFO, 4002)));
    ASSERT_EQ(InformationId::ID_LOCATION_INFO_GPS_LOCATION,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_LOCATION_INFO, 4003)));
    ASSERT_EQ(InformationId::ID_LOCATION_INFO_ZIP_CODE,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_LOCATION_INFO, 4004)));
    ASSERT_EQ(InformationId::ID_LOCATION_INFO_STATE,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_LOCATION_INFO, 4005)));
    ASSERT_EQ(InformationId::ID_LOCATION_INFO_TIMEZONE,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_LOCATION_INFO, 4006)));

    ASSERT_EQ(InformationId::ID_FILE_SYSTEM_USAGE,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_FILE_SYSTEM, 5001)));
    ASSERT_EQ(InformationId::ID_FILE_SYSTEM_CAPACITY,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_FILE_SYSTEM, 5002)));
    ASSERT_EQ(InformationId::ID_FILE_SYSTEM_FREE,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_FILE_SYSTEM, 5003)));
    ASSERT_EQ(InformationId::ID_FILE_SYSTEM_AVAILABLE,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_FILE_SYSTEM, 5004)));

    ASSERT_EQ(InformationId::ID_FORTUNE_FORTUNE,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_FORTUNE, 6001)));

    ASSERT_EQ(InformationId::ID_LAST_LOGIN_LOGIN_SUMMARY,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_LAST_LOGIN, 7001)));
    ASSERT_EQ(InformationId::ID_LAST_LOGIN_LOGIN_TIME,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_LAST_LOGIN, 7002)));
    ASSERT_EQ(InformationId::ID_LAST_LOGIN_LOGOUT_TIME,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_LAST_LOGIN, 7003)));

    ASSERT_EQ(InformationId::ID_PROCESSOR_COUNT_PROCESSOR_CORE_COUNT,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_PROCESSOR_COUNT, 8001)));

    ASSERT_EQ(InformationId::ID_LOAD_AVERAGE_LOAD_AVERAGE,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_LOAD_AVERAGE, 9001)));

    ASSERT_EQ(InformationId::ID_MEMORY_USAGE_TOTAL,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_MEMORY_USAGE, 10001)));
    ASSERT_EQ(InformationId::ID_MEMORY_USAGE_PERCENT_USED,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_MEMORY_USAGE, 10002)));
    ASSERT_EQ(InformationId::ID_MEMORY_USAGE_FREE,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_MEMORY_USAGE, 10003)));

    ASSERT_EQ(InformationId::ID_PROCESSES_PROCESS_COUNT,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_PROCESSES, 11001)));

    ASSERT_EQ(InformationId::ID_SWAP_USAGE_TOTAL,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_SWAP_USAGE, 12001)));
    ASSERT_EQ(InformationId::ID_SWAP_USAGE_PERCENT_USED,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_SWAP_USAGE, 12002)));
    ASSERT_EQ(InformationId::ID_SWAP_USAGE_FREE,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_SWAP_USAGE, 12003)));
    ASSERT_EQ(InformationId::ID_SWAP_USAGE_ENCRYPTED,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_SWAP_USAGE, 12004)));

    ASSERT_EQ(InformationId::ID_SYSTEM_INFORMATION_HOST_NAME,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13001)));
    ASSERT_EQ(InformationId::ID_SYSTEM_INFORMATION_COMPUTER_NAME,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13002)));
    ASSERT_EQ(InformationId::ID_SYSTEM_INFORMATION_KERNEL_VERSION,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13003)));
    ASSERT_EQ(InformationId::ID_SYSTEM_INFORMATION_KERNEL_RELEASE,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13004)));
    ASSERT_EQ(InformationId::ID_SYSTEM_INFORMATION_KERNEL_TYPE,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13005)));
    ASSERT_EQ(InformationId::ID_SYSTEM_INFORMATION_ARCHITECTURE,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13006)));
    ASSERT_EQ(InformationId::ID_SYSTEM_INFORMATION_BYTEORDER,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13007)));
    ASSERT_EQ(InformationId::ID_SYSTEM_INFORMATION_PLATFORM_VERSION,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13008)));
    ASSERT_EQ(InformationId::ID_SYSTEM_INFORMATION_PLATFORM_NAME,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13009)));

    ASSERT_EQ(InformationId::ID_LOGGED_IN_USER_LOGGED_IN,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_LOGGED_IN, 14001)));

    ASSERT_EQ(InformationId::ID_WEATHER_WEATHER,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_WEATHER, 15001)));
    ASSERT_EQ(InformationId::ID_WEATHER_LOCATION,
              static_cast<InformationId>(MakeInformationId(CategoryId::ID_WEATHER, 15002)));
}

} // namespace mmotd::test
