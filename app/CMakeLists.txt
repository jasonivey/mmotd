# mmotd/app/CMakeLists.txt
cmake_minimum_required (VERSION 3.8)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

include (pre_project)

set (MMOTD_TARGET_NAME mmotd)

project (mmotd)

add_executable (
    ${MMOTD_TARGET_NAME}
    src/cli_app_options_creator.cpp
    src/logging.cpp
    src/main.cpp
    )

find_library(FWCoreFoundation NAMES CoreFoundation)
find_library(FWSecurity NAMES Security)

include (target_common)

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)

target_include_directories(
    ${MMOTD_TARGET_NAME}
    PRIVATE ${PROJECT_ROOT_INCLUDE_PATH}
    )

target_include_directories(
    ${MMOTD_TARGET_NAME} SYSTEM
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE ${fmt_SOURCE_DIR}/include
    PRIVATE ${plog_SOURCE_DIR}/include
    PRIVATE ${cli11_SOURCE_DIR}/include
    PRIVATE ${scope_guard_SOURCE_DIR}
    )

# This should include all sub-directory static libraries or shared libraries
target_link_libraries(
    ${MMOTD_TARGET_NAME}
    PRIVATE mmotd_common
    PRIVATE mmotd_lib
    PRIVATE mmotd_view
    )

target_link_libraries(
    ${MMOTD_TARGET_NAME}
    PRIVATE ${FWCoreFoundation}
    PRIVATE ${FWSecurity}
    PRIVATE ${OPENSSL_CRYPTO_LIBRARY}
    PRIVATE ${OPENSSL_SSL_LIBRARY}
    PRIVATE ZLIB::ZLIB
    )
