# mmotd/test/CMakeLists.txt
cmake_minimum_required (VERSION 3.8)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake ${catch2_SOURCE_DIR}/contrib)

include (pre_project)

set (MMOTD_TARGET_NAME mmotd_test)

project (mmotd_test)

add_executable(${MMOTD_TARGET_NAME}
               src/main.cpp
               ../common/test/src/test_assertion.cpp
               ../common/test/src/test_exception.cpp
               ../common/test/src/test_mac_address.cpp
               ../common/test/src/test_template_string.cpp
               ../lib/test/src/test_information_definitions.cpp)

setup_target_properties (${MMOTD_TARGET_NAME})

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)

add_cmake_c_cxx_include_directory(${PROJECT_ROOT_INCLUDE_PATH} FALSE)
add_cmake_c_cxx_include_directory(${Boost_INCLUDE_DIRS} TRUE)
add_cmake_c_cxx_include_directory(${catch2_SOURCE_DIR}/single_include TRUE)
add_cmake_c_cxx_include_directory(${fmt_SOURCE_DIR}/include TRUE)
add_cmake_c_cxx_include_directory(${plog_SOURCE_DIR}/include TRUE)
add_cmake_c_cxx_include_directory(${json_SOURCE_DIR}/include TRUE)
add_cmake_c_cxx_include_directory(${scope_guard_SOURCE_DIR} TRUE)

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
    PRIVATE Catch2::Catch2
    )

include(CTest)
include(ParseAndAddCatchTests)

# In cmake 3.19.6 and higher it complains if this policy is not explicity set since the
#  default naming scheme used by Catch is a descriptive sentence.
cmake_policy(SET CMP0110 NEW)

ParseAndAddCatchTests(${MMOTD_TARGET_NAME})
