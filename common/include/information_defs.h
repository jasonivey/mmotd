// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
CATEGORY_INFO_DEF(GENERAL, general, 110)
CATEGORY_INFO_DEF(NETWORK_INFO, network info, 111)
CATEGORY_INFO_DEF(BOOT_TIME, boot time, 112)
CATEGORY_INFO_DEF(EXTERNAL_NETWORK_INFO, external network info, 113)
CATEGORY_INFO_DEF(LOCATION_INFO, location info, 114)
CATEGORY_INFO_DEF(FILE_SYSTEM, file system, 115)
CATEGORY_INFO_DEF(FORTUNE, fortune, 116)
CATEGORY_INFO_DEF(LAST_LOGIN, last login, 117)
CATEGORY_INFO_DEF(PROCESSOR_COUNT, processor count, 118)
CATEGORY_INFO_DEF(LOAD_AVERAGE, load average, 119)
CATEGORY_INFO_DEF(MEMORY_USAGE, memory usage, 120)
CATEGORY_INFO_DEF(PROCESSES, processes, 121)
CATEGORY_INFO_DEF(SWAP_USAGE, swap usage, 122)
CATEGORY_INFO_DEF(SYSTEM_INFORMATION, system information, 123)
CATEGORY_INFO_DEF(LOGGED_IN, logged in, 124)
CATEGORY_INFO_DEF(WEATHER, weather, 125)

INFO_DEF(GENERAL, GREETING, "greeting", "{}", std::string, 901)
INFO_DEF(GENERAL, USER_NAME, "user name", "{}", std::string, 902)
INFO_DEF(GENERAL, LOCAL_DATE_TIME, "local date time", "{:%a, %d-%h-%Y %I:%M:%S%p %Z}", std::string, 903)
INFO_DEF(GENERAL, LOCAL_TIME_EMOJI, "local time emoji", "{}", std::string, 904)

INFO_DEF(NETWORK_INFO, IP, "ip", "{}", std::string, 1001)
INFO_DEF(NETWORK_INFO, MAC, "mac", "{}", std::string, 1002)
INFO_DEF(NETWORK_INFO, INTERFACE_NAME, "interface name", "{}", std::string, 1003)

INFO_DEF(BOOT_TIME, BOOT_TIME, "boot time", "{}", std::string, 2001)

INFO_DEF(EXTERNAL_NETWORK_INFO, EXTERNAL_IP, "external ip", "{}", std::string, 3001)

INFO_DEF(LOCATION_INFO, CITY, "city", "{}", std::string, 4001)
INFO_DEF(LOCATION_INFO, COUNTRY, "country", "{}", std::string, 4002)
INFO_DEF(LOCATION_INFO, GPS_LOCATION, "gps location", "{}", std::string, 4003)
INFO_DEF(LOCATION_INFO, ZIP_CODE, "zip code", "{}", std::string, 4004)
INFO_DEF(LOCATION_INFO, STATE, "state", "{}", std::string, 4005)
INFO_DEF(LOCATION_INFO, TIMEZONE, "timezone", "{}", std::string, 4006)

INFO_DEF(FILE_SYSTEM, USAGE, "usage", "{:.01f}% of {}", std::string, 5001)
INFO_DEF(FILE_SYSTEM, CAPACITY, "capacity", "{}", uint64_t, 5002)
INFO_DEF(FILE_SYSTEM, FREE, "free", "{}", uint64_t, 5003)
INFO_DEF(FILE_SYSTEM, AVAILABLE, "available", "{}", uint64_t, 5004)

INFO_DEF(FORTUNE, FORTUNE, "", "{}", std::string, 6001)

INFO_DEF(LAST_LOGIN, LOGIN_SUMMARY, "login summary", "{}", std::string, 7001)
INFO_DEF(LAST_LOGIN, LOGIN_TIME, "login time", "{}", std::string, 7002)
INFO_DEF(LAST_LOGIN, LOGOUT_TIME, "logout time", "{}", std::string, 7003)

INFO_DEF(PROCESSOR_COUNT, PROCESSOR_CORE_COUNT, "processor count", "{}", int32_t, 8001)

INFO_DEF(LOAD_AVERAGE, LOAD_AVERAGE, "load average", "{:.02f}", double, 9001)

INFO_DEF(MEMORY_USAGE, TOTAL, "total", "{}", uint64_t, 10001)
INFO_DEF(MEMORY_USAGE, PERCENT_USED, "percent used", "{:.02f}% of {}", double, 10002)
INFO_DEF(MEMORY_USAGE, FREE, "free", "{}", uint64_t, 10003)

INFO_DEF(PROCESSES, PROCESS_COUNT, "process count", "{}", size_t, 11001)

INFO_DEF(SWAP_USAGE, TOTAL, "total", "{}", uint64_t, 12001)
INFO_DEF(SWAP_USAGE, PERCENT_USED, "precent used", "{:.01f}%", double, 12002)
INFO_DEF(SWAP_USAGE, FREE, "free", "{}", uint64_t, 12003)
INFO_DEF(SWAP_USAGE, ENCRYPTED, "encrypted", "{}", bool, 12004)

INFO_DEF(SYSTEM_INFORMATION, HOST_NAME, "hostname", "{}", std::string, 13001)
INFO_DEF(SYSTEM_INFORMATION, COMPUTER_NAME, "computer name", "{}", std::string, 13002)
INFO_DEF(SYSTEM_INFORMATION, KERNEL_VERSION, "kernel version", "{}", std::string, 13003)
INFO_DEF(SYSTEM_INFORMATION, KERNEL_RELEASE, "kernel release", "{}", std::string, 13004)
INFO_DEF(SYSTEM_INFORMATION, KERNEL_TYPE, "kernel type", "{}", std::string, 13005)
INFO_DEF(SYSTEM_INFORMATION, ARCHITECTURE, "architecture", "{}", std::string, 13006)
INFO_DEF(SYSTEM_INFORMATION, BYTEORDER, "byte order", "{}", std::string, 13007)
INFO_DEF(SYSTEM_INFORMATION, PLATFORM_VERSION, "platform version", "{}", std::string, 13008)
INFO_DEF(SYSTEM_INFORMATION, PLATFORM_NAME, "platform name", "{}", std::string, 13009)

INFO_DEF(LOGGED_IN, USER_LOGGED_IN, "users logged in", "{}", std::string, 14001)

INFO_DEF(WEATHER, WEATHER, "weather", "{}", std::string, 15001)
INFO_DEF(WEATHER, LOCATION, "location", "{}", std::string, 15002)
INFO_DEF(WEATHER, SUNRISE, "sunrise", "{}", std::string, 15003)
INFO_DEF(WEATHER, SUNSET, "sunset", "{}", std::string, 15004)

#undef INFO_DEF
#undef CATEGORY_INFO_DEF
