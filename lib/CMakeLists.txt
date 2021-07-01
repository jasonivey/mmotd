# mmotd/lib/CMakeLists.txt
cmake_minimum_required (VERSION 3.8)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

set (MMOTD_TARGET_NAME mmotd_lib)

project (mmotd_lib)

add_library (${MMOTD_TARGET_NAME} STATIC
    src/boot_time.cpp
    src/computer_information.cpp
    src/external_network.cpp
    src/file_system.cpp
    src/fortune.cpp
    src/general.cpp
    src/hardware_information.cpp
    src/http_request.cpp
    src/information_provider.cpp
    src/lastlog.cpp
    src/load_average.cpp
    src/memory.cpp
    src/network.cpp
    src/package_management.cpp
    src/platform/$<LOWER_CASE:$<PLATFORM_ID>>/boot_time.cpp
    src/platform/$<LOWER_CASE:$<PLATFORM_ID>>/hardware_information.cpp
    src/platform/$<LOWER_CASE:$<PLATFORM_ID>>/lastlog.cpp
    src/platform/$<LOWER_CASE:$<PLATFORM_ID>>/load_average.cpp
    src/platform/$<LOWER_CASE:$<PLATFORM_ID>>/memory.cpp
    src/platform/$<LOWER_CASE:$<PLATFORM_ID>>/network.cpp
    src/platform/$<LOWER_CASE:$<PLATFORM_ID>>/package_management.cpp
    src/platform/$<LOWER_CASE:$<PLATFORM_ID>>/processes.cpp
    src/platform/$<LOWER_CASE:$<PLATFORM_ID>>/swap.cpp
    src/platform/$<LOWER_CASE:$<PLATFORM_ID>>/system_information.cpp
    src/platform/$<LOWER_CASE:$<PLATFORM_ID>>/user_accounting_database.cpp
    src/platform/hardware_information.cpp
    src/platform/system_information.cpp
    src/processes.cpp
    src/swap.cpp
    src/system_details.cpp
    src/system_information.cpp
    src/users_logged_in.cpp
    src/weather_info.cpp
    )

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)

setup_target_properties (${MMOTD_TARGET_NAME} ${PROJECT_ROOT_INCLUDE_PATH})
