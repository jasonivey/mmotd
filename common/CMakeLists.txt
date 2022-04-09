# mmotd/common/CMakeLists.txt
cmake_minimum_required (VERSION 3.18)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

set (MMOTD_TARGET_NAME mmotd_common)

project (mmotd_common)

add_library (
    ${MMOTD_TARGET_NAME} STATIC
    assertion/src/exception.cpp
    assertion/src/stack_trace.cpp
    assertion/src/throw.cpp
    src/cli_options_parser.cpp
    src/config_options.cpp
    src/global_state.cpp
    src/information_decls.cpp
    src/information_definitions.cpp
    src/information.cpp
    src/logging.cpp
    src/mac_address.cpp
    src/network_device.cpp
    src/output_template_writer.cpp
    src/output_template.cpp
    src/source_location_common.cpp
    src/source_location.cpp
    src/special_files.cpp
    src/string_utils.cpp
    src/system_command.cpp
    src/template_column_items.cpp
    src/user_information.cpp
    src/version.cpp
    )

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)

setup_target_properties (${MMOTD_TARGET_NAME} ${PROJECT_ROOT_INCLUDE_PATH})
