# mmotd/test/CMakeLists.txt
cmake_minimum_required (VERSION 3.8)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake ${catch2_SOURCE_DIR}/contrib)

set (MMOTD_TARGET_NAME mmotd_test)

project (mmotd_test)

add_executable(${MMOTD_TARGET_NAME}
               src/main.cpp
               ../common/test/src/test_assertion.cpp
               ../common/test/src/test_exception.cpp
               ../common/test/src/test_mac_address.cpp
               ../common/test/src/test_template_string.cpp
               ../lib/test/src/test_information_definitions.cpp)

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)

setup_target_properties (${MMOTD_TARGET_NAME} ${PROJECT_ROOT_INCLUDE_PATH})

include(CTest)
include(ParseAndAddCatchTests)

# In cmake 3.19.6 and higher it complains if this policy is not explicity set since the
#  default naming scheme used by Catch is a descriptive sentence.
cmake_policy(SET CMP0110 NEW)

ParseAndAddCatchTests(${MMOTD_TARGET_NAME})
