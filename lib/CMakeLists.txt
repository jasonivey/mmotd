# mmotd/lib/CMakeLists.txt
cmake_minimum_required (VERSION 3.8)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

include (pre_project)

set (MMOTD_TARGET_NAME mmotd_lib)

project (mmotd_lib)

add_library (
    ${MMOTD_TARGET_NAME} STATIC
    src/boot_time.cpp
    src/computer_information.cpp
    src/external_network.cpp
    src/file_system.cpp
    src/fortune.cpp
    src/http_request.cpp
    src/http_secure_request.cpp
    src/lastlog.cpp
    src/load_average.cpp
    src/mac_address.cpp
    src/memory.cpp
    src/network.cpp
    src/platform/darwin/boot_time.cpp
    src/platform/darwin/lastlog.cpp
    src/platform/darwin/load_average.cpp
    src/platform/darwin/memory.cpp
    src/platform/darwin/network.cpp
    src/platform/darwin/processes.cpp
    src/platform/darwin/swap.cpp
    src/platform/darwin/system_information.cpp
    src/platform/darwin/user_accounting_database.cpp
    src/platform/linux/boot_time.cpp
    src/platform/linux/lastlog.cpp
    src/platform/linux/load_average.cpp
    src/platform/linux/memory.cpp
    src/platform/linux/network.cpp
    src/platform/linux/processes.cpp
    src/platform/linux/swap.cpp
    src/platform/linux/system_information.cpp
    src/platform/linux/user_accounting_database.cpp
    src/platform/windows/boot_time.cpp
    src/platform/windows/lastlog.cpp
    src/platform/windows/load_average.cpp
    src/platform/windows/memory.cpp
    src/platform/windows/network.cpp
    src/platform/windows/processes.cpp
    src/platform/windows/swap.cpp
    src/platform/windows/system_information.cpp
    src/platform/windows/user_accounting_database.cpp
    src/posix_sysctl_information.cpp
    src/processes.cpp
    src/swap.cpp
    src/system_details.cpp
    src/system_information.cpp
    src/users_logged_in.cpp
    src/weather_info.cpp
    )

include (target_common)

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)

target_include_directories(
    ${MMOTD_TARGET_NAME}
    PRIVATE ${PROJECT_ROOT_INCLUDE_PATH}
    )

target_include_directories(
    ${MMOTD_TARGET_NAME} SYSTEM
    PRIVATE ${OPENSSL_INCLUDE_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE ${scope_guard_SOURCE_DIR}
    PRIVATE ${certify_SOURCE_DIR}/include
    PRIVATE ${fmt_SOURCE_DIR}/include
    PRIVATE ${plog_SOURCE_DIR}/include
    PRIVATE ${random_SOURCE_DIR}/include
    )

