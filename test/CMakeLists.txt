# mmotd/test/CMakeLists.txt
cmake_minimum_required (VERSION 3.18)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake ${catch2_SOURCE_DIR}/contrib)

set (MMOTD_TARGET_NAME mmotd_test)

project (mmotd_test)

include (set_policies)
set_default_policies()

add_executable(${MMOTD_TARGET_NAME}
               ../common/test/src/exception_matcher.cpp
               ../common/test/src/test_algorithm.cpp
               ../common/test/src/test_assertion.cpp
               ../common/test/src/test_config_options.cpp
               ../common/test/src/test_exception.cpp
               ../common/test/src/test_mac_address.cpp
               ../common/test/src/test_output_template.cpp
               ../common/test/src/test_output_template_writer.cpp
               ../common/test/src/test_special_files.cpp
               ../lib/test/src/test_information_definitions.cpp
               src/main.cpp
              )

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)

setup_target_properties (${MMOTD_TARGET_NAME} ${PROJECT_ROOT_INCLUDE_PATH})

include(CTest)
include(ParseAndAddCatchTests)

ParseAndAddCatchTests(${MMOTD_TARGET_NAME})

install(TARGETS "${MMOTD_TARGET_NAME}"
        CONFIGURATIONS "${CMAKE_BUILD_TYPE}"
        RUNTIME DESTINATION "${CMAKE_BUILD_TYPE}/bin")
