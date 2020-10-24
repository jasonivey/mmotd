# mmotd/app/CMakeLists.txt
cmake_minimum_required (VERSION 3.10)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

set (MMOTD_TARGET_NAME mmotd_info)

project (mmotd-info)

add_executable (
    ${MMOTD_TARGET_NAME}
    src/cli_app_options_creator.cpp
    src/main.cpp
    )

find_library(FWCoreFoundation NAMES CoreFoundation)
find_library(FWSecurity NAMES Security)

include (common)

target_include_directories(
    ${MMOTD_TARGET_NAME} SYSTEM
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE ${OPENSSL_INCLUDE_DIR}
    PRIVATE ${JSONCPP_INCLUDE_DIRS}
    )

target_include_directories(
    ${MMOTD_TARGET_NAME} BEFORE
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_SOURCE_DIR}/lib/include
    )

target_link_libraries(
    ${MMOTD_TARGET_NAME}
    PRIVATE mmotd
    PRIVATE ${FWCoreFoundation}
    PRIVATE ${FWSecurity}
    PRIVATE ${Boost_LIBRARIES}
    PRIVATE fmt::fmt
    PRIVATE ${JSONCPP_LINK_LIBRARIES}
    PRIVATE ${OPENSSL_CRYPTO_LIBRARY}
    PRIVATE ${OPENSSL_SSL_LIBRARY}
    PRIVATE ZLIB::ZLIB
    PRIVATE CLI11::CLI11
    )
