# mmotd/common/CMakeLists.txt
cmake_minimum_required (VERSION 3.8)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

include (pre_project)

set (MMOTD_TARGET_NAME mmotd_common)

project (mmotd_common)

include (git_version)

add_library (
    ${MMOTD_TARGET_NAME} STATIC
    assertion/src/assertion.cpp
    assertion/src/exception.cpp
    assertion/src/stack_trace.cpp
    src/app_options.cpp
    src/information.cpp
    src/information_decls.cpp
    src/information_definitions.cpp
    src/information_objects.cpp
    src/informations.cpp
    src/logging.cpp
    src/mac_address.cpp
    src/network_device.cpp
    src/tty_template.cpp
    src/tty_template_data.cpp
    src/tty_template_io.cpp
    src/tty_template_string.cpp
    src/tty_template_substring.cpp
    src/tty_template_substring_range.cpp
    src/tty_template_substrings.cpp
    src/version.cpp
    )

setup_target_properties (${MMOTD_TARGET_NAME})

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)

add_cmake_c_cxx_include_directory(${PROJECT_ROOT_INCLUDE_PATH} FALSE)
add_cmake_c_cxx_include_directory(${json_SOURCE_DIR}/include TRUE)
add_cmake_c_cxx_include_directory(${Boost_INCLUDE_DIRS} TRUE)
add_cmake_c_cxx_include_directory(${scope_guard_SOURCE_DIR} TRUE)
add_cmake_c_cxx_include_directory(${fmt_SOURCE_DIR}/include TRUE)
add_cmake_c_cxx_include_directory(${plog_SOURCE_DIR}/include TRUE)

