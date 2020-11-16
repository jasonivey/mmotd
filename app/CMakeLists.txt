# mmotd/app/CMakeLists.txt
cmake_minimum_required (VERSION 3.18)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

include (pre_project)

set (MMOTD_TARGET_NAME mmotd_info)

project (mmotd_info)

add_executable (
    ${MMOTD_TARGET_NAME}
    src/cli_app_options_creator.cpp
    src/color.cpp
    src/logging.cpp
    src/main.cpp
    )

find_library(FWCoreFoundation NAMES CoreFoundation)
find_library(FWSecurity NAMES Security)

include (common)

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)
message(STATUS "adding include directory: ${PROJECT_ROOT_INCLUDE_PATH}")

target_include_directories(
    ${MMOTD_TARGET_NAME}
    PRIVATE ${PROJECT_ROOT_INCLUDE_PATH}
    )

target_include_directories(
    ${MMOTD_TARGET_NAME} SYSTEM
    PRIVATE ${CMAKE_SOURCE_DIR}/external/plog/include
    PRIVATE ${CMAKE_SOURCE_DIR}/external/CLI11/include
    PRIVATE ${Boost_INCLUDE_DIRS}
    )

# This should include all sub-directory static libraries or shared libraries
target_link_libraries(
    ${MMOTD_TARGET_NAME}
    PRIVATE mmotd
    PRIVATE mmotd_view
    )

target_link_libraries(
    ${MMOTD_TARGET_NAME}
    PRIVATE ${FWCoreFoundation}
    PRIVATE ${FWSecurity}
    PRIVATE fmt::fmt-header-only
    PRIVATE ${OPENSSL_CRYPTO_LIBRARY}
    PRIVATE ${OPENSSL_SSL_LIBRARY}
    PRIVATE ZLIB::ZLIB
    )

