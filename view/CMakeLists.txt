# mmotd/view/CMakeLists.txt
cmake_minimum_required (VERSION 3.18)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

include (pre_project)

set (MMOTD_TARGET_NAME mmotd_view)

project (mmotd_view)

add_library (
    ${MMOTD_TARGET_NAME} STATIC
    src/active_network_interfaces.cpp
    src/computer_information_provider.cpp
    src/computer_name.cpp
    src/disk_usage.cpp
    src/computer_information_provider_factory.cpp
    src/greeting.cpp
    src/header.cpp
    src/host_name.cpp
    src/last_login.cpp
    src/memory_usage.cpp
    src/processor_count.cpp
    src/public_ip.cpp
    src/random_quote.cpp
    src/sub_header.cpp
    src/swap_usage.cpp
    src/system_load.cpp
    src/unread_mail.cpp
    src/users_count.cpp
    )

include (target_common)

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)

target_include_directories(
    ${MMOTD_TARGET_NAME}
    PRIVATE ${PROJECT_ROOT_INCLUDE_PATH}
    )

target_include_directories(
    ${MMOTD_TARGET_NAME} SYSTEM
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE ${scope_guard_SOURCE_DIR}
    PRIVATE ${fmt_SOURCE_DIR}/include
    PRIVATE ${plog_SOURCE_DIR}/include
    )

