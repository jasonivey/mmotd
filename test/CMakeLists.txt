# mmotd/test/CMakeLists.txt
cmake_minimum_required (VERSION 3.8)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

include (pre_project)

set (MMOTD_TARGET_NAME mmotd_test)

project (mmotd_test)

include (GoogleTest)

add_executable(${MMOTD_TARGET_NAME}
               src/main.cpp
               ../common/test/src/test_mac_address.cpp
               ../common/test/src/test_template_string.cpp
               ../lib/test/src/test_information_definitions.cpp)

include (target_common)

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)

target_include_directories(
    ${MMOTD_TARGET_NAME}
    PRIVATE ${PROJECT_ROOT_INCLUDE_PATH}
    )

target_include_directories(
    ${MMOTD_TARGET_NAME} SYSTEM
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE ${ranges_SOURCE_DIR}/include
    PRIVATE ${fmt_SOURCE_DIR}/include
    PRIVATE ${plog_SOURCE_DIR}/include
    PRIVATE ${json_SOURCE_DIR}/include
    PRIVATE ${scope_guard_SOURCE_DIR}
    )

target_link_libraries(
    ${MMOTD_TARGET_NAME}
    PRIVATE mmotd_common
    PRIVATE mmotd_lib
    )

target_link_libraries(
    ${MMOTD_TARGET_NAME}
    PRIVATE ${OPENSSL_CRYPTO_LIBRARY}
    PRIVATE ${OPENSSL_SSL_LIBRARY}
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE ZLIB::ZLIB
    PRIVATE Threads::Threads
    gtest
    gmock_main
    )

gtest_discover_tests(${MMOTD_TARGET_NAME})
