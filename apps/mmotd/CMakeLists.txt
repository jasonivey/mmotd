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
    src/main.cpp
    )

if (APPLE)
    find_library(FWCoreFoundation NAMES CoreFoundation)
    find_library(FWSecurity NAMES Security)
endif ()

include (target_common)

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)

add_cmake_c_cxx_include_directory(${PROJECT_ROOT_INCLUDE_PATH} FALSE)
add_cmake_c_cxx_include_directory(${Boost_INCLUDE_DIRS} TRUE)
add_cmake_c_cxx_include_directory(${ranges_SOURCE_DIR}/include TRUE)
add_cmake_c_cxx_include_directory(${fmt_SOURCE_DIR}/include TRUE)
add_cmake_c_cxx_include_directory(${plog_SOURCE_DIR}/include TRUE)
add_cmake_c_cxx_include_directory(${cli11_SOURCE_DIR}/include TRUE)
add_cmake_c_cxx_include_directory(${json_SOURCE_DIR}/include TRUE)
add_cmake_c_cxx_include_directory(${scope_guard_SOURCE_DIR} TRUE)

# This should include all sub-directory static libraries or shared libraries
target_link_libraries(
    ${MMOTD_TARGET_NAME}
    PRIVATE mmotd_lib
    PRIVATE mmotd_common
    )

if (APPLE)
    target_link_libraries(
        ${MMOTD_TARGET_NAME}
        PRIVATE ${FWCoreFoundation}
        PRIVATE ${FWSecurity}
        )
endif ()

target_link_libraries(
    ${MMOTD_TARGET_NAME}
    PRIVATE ${OPENSSL_CRYPTO_LIBRARY}
    PRIVATE ${OPENSSL_SSL_LIBRARY}
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE ZLIB::ZLIB
    PRIVATE Threads::Threads
    )
