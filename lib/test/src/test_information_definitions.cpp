#include "common/include/information_definitions.h"

#include <catch2/catch.hpp>

namespace mmotd::test {

CATCH_TEST_CASE("category ids are correct", "[CategoryId]") {
    using namespace mmotd::information;

    CATCH_CHECK(CategoryId::ID_GENERAL == static_cast<CategoryId>(MakeCategoryId(110)));
    CATCH_CHECK(CategoryId::ID_NETWORK_INFO == static_cast<CategoryId>(MakeCategoryId(111)));
    CATCH_CHECK(CategoryId::ID_BOOT_TIME == static_cast<CategoryId>(MakeCategoryId(112)));
    CATCH_CHECK(CategoryId::ID_EXTERNAL_NETWORK_INFO == static_cast<CategoryId>(MakeCategoryId(113)));
    CATCH_CHECK(CategoryId::ID_LOCATION_INFO == static_cast<CategoryId>(MakeCategoryId(114)));
    CATCH_CHECK(CategoryId::ID_FILE_SYSTEM == static_cast<CategoryId>(MakeCategoryId(115)));
    CATCH_CHECK(CategoryId::ID_FORTUNE == static_cast<CategoryId>(MakeCategoryId(116)));
    CATCH_CHECK(CategoryId::ID_LAST_LOGIN == static_cast<CategoryId>(MakeCategoryId(117)));
    CATCH_CHECK(CategoryId::ID_LOAD_AVERAGE == static_cast<CategoryId>(MakeCategoryId(119)));
    CATCH_CHECK(CategoryId::ID_MEMORY_USAGE == static_cast<CategoryId>(MakeCategoryId(120)));
    CATCH_CHECK(CategoryId::ID_PROCESSES == static_cast<CategoryId>(MakeCategoryId(121)));
    CATCH_CHECK(CategoryId::ID_SWAP_USAGE == static_cast<CategoryId>(MakeCategoryId(122)));
    CATCH_CHECK(CategoryId::ID_SYSTEM_INFORMATION == static_cast<CategoryId>(MakeCategoryId(123)));
    CATCH_CHECK(CategoryId::ID_LOGGED_IN == static_cast<CategoryId>(MakeCategoryId(124)));
    CATCH_CHECK(CategoryId::ID_WEATHER == static_cast<CategoryId>(MakeCategoryId(125)));
    CATCH_CHECK(CategoryId::ID_PACKAGE_MANAGEMENT == static_cast<CategoryId>(MakeCategoryId(126)));
    CATCH_CHECK(CategoryId::ID_HARDWARE == static_cast<CategoryId>(MakeCategoryId(127)));

    CATCH_CHECK(CategoryId::ID_GENERAL == static_cast<CategoryId>(0x6Ellu << 32));
    CATCH_CHECK(CategoryId::ID_NETWORK_INFO == static_cast<CategoryId>(0x6Fllu << 32));
    CATCH_CHECK(CategoryId::ID_BOOT_TIME == static_cast<CategoryId>(0x70llu << 32));
    CATCH_CHECK(CategoryId::ID_EXTERNAL_NETWORK_INFO == static_cast<CategoryId>(0x71llu << 32));
    CATCH_CHECK(CategoryId::ID_LOCATION_INFO == static_cast<CategoryId>(0x72llu << 32));
    CATCH_CHECK(CategoryId::ID_FILE_SYSTEM == static_cast<CategoryId>(0x73llu << 32));
    CATCH_CHECK(CategoryId::ID_FORTUNE == static_cast<CategoryId>(0x74llu << 32));
    CATCH_CHECK(CategoryId::ID_LAST_LOGIN == static_cast<CategoryId>(0x75llu << 32));
    CATCH_CHECK(CategoryId::ID_LOAD_AVERAGE == static_cast<CategoryId>(0x77llu << 32));
    CATCH_CHECK(CategoryId::ID_MEMORY_USAGE == static_cast<CategoryId>(0x78llu << 32));
    CATCH_CHECK(CategoryId::ID_PROCESSES == static_cast<CategoryId>(0x79llu << 32));
    CATCH_CHECK(CategoryId::ID_SWAP_USAGE == static_cast<CategoryId>(0x7Allu << 32));
    CATCH_CHECK(CategoryId::ID_SYSTEM_INFORMATION == static_cast<CategoryId>(0x7Bllu << 32));
    CATCH_CHECK(CategoryId::ID_LOGGED_IN == static_cast<CategoryId>(0x7Cllu << 32));
    CATCH_CHECK(CategoryId::ID_WEATHER == static_cast<CategoryId>(0x7Dllu << 32));
    CATCH_CHECK(CategoryId::ID_PACKAGE_MANAGEMENT == static_cast<CategoryId>(0x7Ellu << 32));
    CATCH_CHECK(CategoryId::ID_HARDWARE == static_cast<CategoryId>(0x7Fllu << 32));
}

CATCH_TEST_CASE("information ids are correct", "[InformationId]") {
    using namespace mmotd::information;

    CATCH_CHECK(InformationId::ID_GENERAL_GREETING ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_GENERAL, 901)));
    CATCH_CHECK(InformationId::ID_GENERAL_USER_NAME ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_GENERAL, 902)));
    CATCH_CHECK(InformationId::ID_GENERAL_LOCAL_DATE_TIME ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_GENERAL, 903)));
    CATCH_CHECK(InformationId::ID_GENERAL_LOCAL_TIME_EMOJI ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_GENERAL, 904)));

    CATCH_CHECK(InformationId::ID_NETWORK_INFO_IP ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_NETWORK_INFO, 1001)));
    CATCH_CHECK(InformationId::ID_NETWORK_INFO_MAC ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_NETWORK_INFO, 1002)));
    CATCH_CHECK(InformationId::ID_NETWORK_INFO_INTERFACE_NAME ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_NETWORK_INFO, 1003)));

    CATCH_CHECK(InformationId::ID_BOOT_TIME_BOOT_TIME ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_BOOT_TIME, 2001)));
    CATCH_CHECK(InformationId::ID_BOOT_TIME_UP_TIME ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_BOOT_TIME, 2002)));

    CATCH_CHECK(InformationId::ID_EXTERNAL_NETWORK_INFO_EXTERNAL_IP ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_EXTERNAL_NETWORK_INFO, 3001)));

    CATCH_CHECK(InformationId::ID_LOCATION_INFO_CITY ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_LOCATION_INFO, 4001)));
    CATCH_CHECK(InformationId::ID_LOCATION_INFO_COUNTRY ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_LOCATION_INFO, 4002)));
    CATCH_CHECK(InformationId::ID_LOCATION_INFO_GPS_LOCATION ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_LOCATION_INFO, 4003)));
    CATCH_CHECK(InformationId::ID_LOCATION_INFO_ZIP_CODE ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_LOCATION_INFO, 4004)));
    CATCH_CHECK(InformationId::ID_LOCATION_INFO_STATE ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_LOCATION_INFO, 4005)));
    CATCH_CHECK(InformationId::ID_LOCATION_INFO_TIMEZONE ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_LOCATION_INFO, 4006)));

    CATCH_CHECK(InformationId::ID_FILE_SYSTEM_PERCENT_USED ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_FILE_SYSTEM, 5001)));
    CATCH_CHECK(InformationId::ID_FILE_SYSTEM_TOTAL ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_FILE_SYSTEM, 5002)));
    CATCH_CHECK(InformationId::ID_FILE_SYSTEM_FREE ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_FILE_SYSTEM, 5003)));
    CATCH_CHECK(InformationId::ID_FILE_SYSTEM_SUMMARY ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_FILE_SYSTEM, 5004)));

    CATCH_CHECK(InformationId::ID_FORTUNE_FORTUNE ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_FORTUNE, 6001)));

    CATCH_CHECK(InformationId::ID_LAST_LOGIN_LOGIN_SUMMARY ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_LAST_LOGIN, 7001)));
    CATCH_CHECK(InformationId::ID_LAST_LOGIN_LOGIN_TIME ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_LAST_LOGIN, 7002)));
    CATCH_CHECK(InformationId::ID_LAST_LOGIN_LOGOUT_TIME ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_LAST_LOGIN, 7003)));

    CATCH_CHECK(InformationId::ID_LOAD_AVERAGE_LOAD_AVERAGE ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_LOAD_AVERAGE, 9001)));

    CATCH_CHECK(InformationId::ID_MEMORY_USAGE_TOTAL ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_MEMORY_USAGE, 10001)));
    CATCH_CHECK(InformationId::ID_MEMORY_USAGE_PERCENT_USED ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_MEMORY_USAGE, 10002)));
    CATCH_CHECK(InformationId::ID_MEMORY_USAGE_FREE ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_MEMORY_USAGE, 10003)));
    CATCH_CHECK(InformationId::ID_MEMORY_USAGE_SUMMARY ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_MEMORY_USAGE, 10004)));

    CATCH_CHECK(InformationId::ID_SWAP_USAGE_TOTAL ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_SWAP_USAGE, 12001)));
    CATCH_CHECK(InformationId::ID_SWAP_USAGE_PERCENT_USED ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_SWAP_USAGE, 12002)));
    CATCH_CHECK(InformationId::ID_SWAP_USAGE_FREE ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_SWAP_USAGE, 12003)));
    CATCH_CHECK(InformationId::ID_SWAP_USAGE_ENCRYPTED ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_SWAP_USAGE, 12004)));
    CATCH_CHECK(InformationId::ID_SWAP_USAGE_SUMMARY ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_SWAP_USAGE, 12005)));

    CATCH_CHECK(InformationId::ID_SYSTEM_INFORMATION_HOST_NAME ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13001)));
    CATCH_CHECK(InformationId::ID_SYSTEM_INFORMATION_COMPUTER_NAME ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13002)));
    CATCH_CHECK(InformationId::ID_SYSTEM_INFORMATION_KERNEL_VERSION ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13003)));
    CATCH_CHECK(InformationId::ID_SYSTEM_INFORMATION_KERNEL_RELEASE ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13004)));
    CATCH_CHECK(InformationId::ID_SYSTEM_INFORMATION_KERNEL_TYPE ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13005)));
    CATCH_CHECK(InformationId::ID_SYSTEM_INFORMATION_ARCHITECTURE ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13006)));
    CATCH_CHECK(InformationId::ID_SYSTEM_INFORMATION_PLATFORM_VERSION ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13007)));
    CATCH_CHECK(InformationId::ID_SYSTEM_INFORMATION_PLATFORM_NAME ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_SYSTEM_INFORMATION, 13008)));

    CATCH_CHECK(InformationId::ID_LOGGED_IN_USER_LOGGED_IN ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_LOGGED_IN, 14001)));

    CATCH_CHECK(InformationId::ID_WEATHER_WEATHER ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_WEATHER, 15001)));
    CATCH_CHECK(InformationId::ID_WEATHER_LOCATION ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_WEATHER, 15002)));

    CATCH_CHECK(InformationId::ID_PACKAGE_MANAGEMENT_UPDATE_DETAILS ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_PACKAGE_MANAGEMENT, 16001)));
    CATCH_CHECK(InformationId::ID_PACKAGE_MANAGEMENT_REBOOT_REQUIRED ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_PACKAGE_MANAGEMENT, 16002)));

    CATCH_CHECK(InformationId::ID_HARDWARE_MACHINE_TYPE ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_HARDWARE, 17001)));
    CATCH_CHECK(InformationId::ID_HARDWARE_MACHINE_MODEL ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_HARDWARE, 17002)));
    CATCH_CHECK(InformationId::ID_HARDWARE_CPU_NAME ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_HARDWARE, 17003)));
    CATCH_CHECK(InformationId::ID_HARDWARE_CPU_CORE_COUNT ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_HARDWARE, 17004)));
    CATCH_CHECK(InformationId::ID_HARDWARE_CPU_BYTE_ORDER ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_HARDWARE, 17005)));
    CATCH_CHECK(InformationId::ID_HARDWARE_GPU_MODEL_NAME ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_HARDWARE, 17006)));
    CATCH_CHECK(InformationId::ID_HARDWARE_MONITOR_NAME ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_HARDWARE, 17007)));
    CATCH_CHECK(InformationId::ID_HARDWARE_MONITOR_RESOLUTION ==
                static_cast<InformationId>(MakeInformationId(CategoryId::ID_HARDWARE, 17008)));
}

} // namespace mmotd::test
