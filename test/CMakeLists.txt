# mmotd/test/CMakeLists.txt
cmake_minimum_required (VERSION 3.8)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake ${catch2_SOURCE_DIR}/contrib)

set (MMOTD_TARGET_NAME mmotd_test)

project (mmotd_test)

include (set_policies)
set_default_policies()

add_executable(${MMOTD_TARGET_NAME}
               ../common/test/src/exception_matcher.cpp
               ../common/test/src/test_assertion.cpp
               ../common/test/src/test_exception.cpp
               ../common/test/src/test_mac_address.cpp
               ../common/test/src/test_position_index.cpp
               ../common/test/src/test_template_string.cpp
               ../lib/test/src/test_information_definitions.cpp
               src/main.cpp
              )

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)

setup_target_properties (${MMOTD_TARGET_NAME} ${PROJECT_ROOT_INCLUDE_PATH})

include(CTest)
include(ParseAndAddCatchTests)

ParseAndAddCatchTests(${MMOTD_TARGET_NAME})
