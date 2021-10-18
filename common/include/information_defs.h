// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
// #include "common/include/information_decls.h"
CATEGORY_INFO_DEF(GENERAL, general, 110)
CATEGORY_INFO_DEF(NETWORK_INFO, network info, 111)
CATEGORY_INFO_DEF(BOOT_TIME, boot time, 112)
CATEGORY_INFO_DEF(EXTERNAL_NETWORK_INFO, external network info, 113)
CATEGORY_INFO_DEF(LOCATION_INFO, location info, 114)
CATEGORY_INFO_DEF(FILE_SYSTEM, file system, 115)
CATEGORY_INFO_DEF(FORTUNE, fortune, 116)
CATEGORY_INFO_DEF(LAST_LOGIN, last login, 117)
CATEGORY_INFO_DEF(LOAD_AVERAGE, load average, 119)
CATEGORY_INFO_DEF(MEMORY_USAGE, memory usage, 120)
CATEGORY_INFO_DEF(PROCESSES, processes, 121)
CATEGORY_INFO_DEF(SWAP_USAGE, swap usage, 122)
CATEGORY_INFO_DEF(SYSTEM_INFORMATION, system information, 123)
CATEGORY_INFO_DEF(LOGGED_IN, logged in, 124)
CATEGORY_INFO_DEF(WEATHER, weather, 125)
CATEGORY_INFO_DEF(PACKAGE_MANAGEMENT, package management, 126)
CATEGORY_INFO_DEF(HARDWARE, hardware, 127)

INFO_DEF(GENERAL, GREETING, "greeting", "{}", 901)
INFO_DEF(GENERAL, USER_NAME, "user name", "{}", 902)
INFO_DEF(GENERAL, LOCAL_DATE_TIME, "local date time", "{}", 903)
INFO_DEF(GENERAL, LOCAL_TIME_EMOJI, "local time emoji", "{}", 904)

INFO_DEF(NETWORK_INFO, IP, "ip", "{}", 1001)
INFO_DEF(NETWORK_INFO, MAC, "mac", "{}", 1002)
INFO_DEF(NETWORK_INFO, INTERFACE_NAME, "interface name", "{}", 1003)

INFO_DEF(BOOT_TIME, BOOT_TIME, "boot time", "{}", 2001)
INFO_DEF(BOOT_TIME, UP_TIME, "uptime", "{}", 2002)

INFO_DEF(EXTERNAL_NETWORK_INFO, EXTERNAL_IP, "external ip", "{}", 3001)

INFO_DEF(LOCATION_INFO, CITY, "city", "{}", 4001)
INFO_DEF(LOCATION_INFO, COUNTRY, "country", "{}", 4002)
INFO_DEF(LOCATION_INFO, GPS_LOCATION, "gps location", "{}", 4003)
INFO_DEF(LOCATION_INFO, ZIP_CODE, "zip code", "{}", 4004)
INFO_DEF(LOCATION_INFO, STATE, "state", "{}", 4005)
INFO_DEF(LOCATION_INFO, TIMEZONE, "timezone", "{}", 4006)

INFO_DEF(FILE_SYSTEM, PERCENT_USED, "percent used", "{:.01f}%", 5001)
INFO_DEF(FILE_SYSTEM, TOTAL, "total", "{}", 5002)
INFO_DEF(FILE_SYSTEM, FREE, "free", "{}", 5003)
INFO_DEF(FILE_SYSTEM, SUMMARY, "summary", "{}", 5004)

INFO_DEF(FORTUNE, FORTUNE, "", "{}", 6001)

INFO_DEF(LAST_LOGIN, LOGIN_SUMMARY, "login summary", "{}", 7001)
INFO_DEF(LAST_LOGIN, LOGIN_TIME, "login time", "{}", 7002)
INFO_DEF(LAST_LOGIN, LOGOUT_TIME, "logout time", "{}", 7003)

INFO_DEF(LOAD_AVERAGE, LOAD_AVERAGE, "load average", "{:.02f}", 9001)

INFO_DEF(MEMORY_USAGE, TOTAL, "total", "{}", 10001)
INFO_DEF(MEMORY_USAGE, PERCENT_USED, "percent used", "{:.01f}%", 10002)
INFO_DEF(MEMORY_USAGE, FREE, "free", "{}", 10003)
INFO_DEF(MEMORY_USAGE, SUMMARY, "summary", "{}", 10004)

INFO_DEF(PROCESSES, PROCESS_COUNT, "process count", "{}", 11001)

INFO_DEF(SWAP_USAGE, TOTAL, "total", "{}", 12001)
INFO_DEF(SWAP_USAGE, PERCENT_USED, "precent used", "{:.01f}%", 12002)
INFO_DEF(SWAP_USAGE, FREE, "free", "{}", 12003)
INFO_DEF(SWAP_USAGE, ENCRYPTED, "encrypted", "{}", 12004)
INFO_DEF(SWAP_USAGE, SUMMARY, "summary", "{}", 12005)

INFO_DEF(SYSTEM_INFORMATION, HOST_NAME, "hostname", "{}", 13001)
INFO_DEF(SYSTEM_INFORMATION, COMPUTER_NAME, "computer name", "{}", 13002)
INFO_DEF(SYSTEM_INFORMATION, KERNEL_VERSION, "kernel version", "{}", 13003)
INFO_DEF(SYSTEM_INFORMATION, KERNEL_RELEASE, "kernel release", "{}", 13004)
INFO_DEF(SYSTEM_INFORMATION, KERNEL_TYPE, "kernel type", "{}", 13005)
INFO_DEF(SYSTEM_INFORMATION, ARCHITECTURE, "architecture", "{}", 13006)
INFO_DEF(SYSTEM_INFORMATION, PLATFORM_VERSION, "platform version", "{}", 13007)
INFO_DEF(SYSTEM_INFORMATION, PLATFORM_NAME, "platform name", "{}", 13008)

INFO_DEF(LOGGED_IN, USER_LOGGED_IN, "users logged in", "{}", 14001)

INFO_DEF(WEATHER, WEATHER, "weather", "{}", 15001)
INFO_DEF(WEATHER, LOCATION, "location", "{}", 15002)
INFO_DEF(WEATHER, SUNRISE, "sunrise", "{}", 15003)
INFO_DEF(WEATHER, SUNSET, "sunset", "{}", 15004)

INFO_DEF(PACKAGE_MANAGEMENT, UPDATE_DETAILS, "", "{}", 16001)
INFO_DEF(PACKAGE_MANAGEMENT, REBOOT_REQUIRED, "", "{}", 16002)

INFO_DEF(HARDWARE, MACHINE_TYPE, "machine", "{}", 17001)
INFO_DEF(HARDWARE, MACHINE_MODEL, "model", "{}", 17002)
INFO_DEF(HARDWARE, CPU_NAME, "cpu name", "{}", 17003)
INFO_DEF(HARDWARE, CPU_CORE_COUNT, "cpu core count", "{}", 17004)
INFO_DEF(HARDWARE, CPU_BYTE_ORDER, "byte order", "{}", 17005)
INFO_DEF(HARDWARE, GPU_MODEL_NAME, "gpu name", "{}", 17006)
INFO_DEF(HARDWARE, MONITOR_NAME, "monitor", "{}", 17007)
INFO_DEF(HARDWARE, MONITOR_RESOLUTION, "resolution", "{}", 17008)

#undef INFO_DEF
#undef CATEGORY_INFO_DEF
