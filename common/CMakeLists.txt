# mmotd/common/CMakeLists.txt
cmake_minimum_required (VERSION 3.8)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

set (MMOTD_TARGET_NAME mmotd_common)

project (mmotd_common)

add_library (
    ${MMOTD_TARGET_NAME} STATIC
    assertion/src/assertion.cpp
    assertion/src/exception.cpp
    assertion/src/stack_trace.cpp
    results/src/output_column.cpp
    results/src/output_column_indexes.cpp
    results/src/output_frame.cpp
    results/src/output_position_index.cpp
    results/src/output_row.cpp
    results/src/output_row_number_sentinals.cpp
    results/src/output_table.cpp
    results/src/output_template.cpp
    results/src/output_template_printer.cpp
    results/src/template_column_items.cpp
    results/src/template_string.cpp
    results/src/template_substring.cpp
    results/src/template_substring_range.cpp
    results/src/template_substrings.cpp
    src/cli_options_parser.cpp
    src/coloring.cpp
    src/config_options.cpp
    src/information.cpp
    src/information_decls.cpp
    src/information_definitions.cpp
    src/information_objects.cpp
    src/informations.cpp
    src/logging.cpp
    src/mac_address.cpp
    src/network_device.cpp
    src/source_location.cpp
    src/source_location_common.cpp
    src/special_files.cpp
    src/system_command.cpp
    src/user_information.cpp
    src/version.cpp
    )

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)

setup_target_properties (${MMOTD_TARGET_NAME} ${PROJECT_ROOT_INCLUDE_PATH})
