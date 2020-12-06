# mmotd/lib/CMakeLists.txt
cmake_minimum_required (VERSION 3.8)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

include (pre_project)

set (MMOTD_TARGET_NAME mmotd_lib)

project (mmotd_lib)

add_library (
    ${MMOTD_TARGET_NAME} STATIC
    src/app_options.cpp
    src/app_options_creator.cpp
    src/boot_time.cpp
    src/computer_information.cpp
    src/external_network.cpp
    src/file_system.cpp
    src/fortune.cpp
    src/http_request.cpp
    src/http_secure_request.cpp
    src/lastlog.cpp
    src/load_average.cpp
    src/memory.cpp
    src/network.cpp
    src/posix_sysctl_information.cpp
    src/posix_system_information.cpp
    src/processes.cpp
    src/swap.cpp
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

