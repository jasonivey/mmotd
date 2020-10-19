# mmotd/lib/CMakeLists.txt
cmake_minimum_required (VERSION 3.10)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

set (MMOTD_TARGET_NAME mmotd)

project (mmotd-library)

add_library (
    ${MMOTD_TARGET_NAME} STATIC
    src/external_network.cpp
    src/http_request.cpp
    src/http_secure_request.cpp
    src/lastlog.cpp
    src/logging.cpp
    src/network.cpp
    src/options.cpp
    src/system.cpp
    )

include (common)

target_include_directories(
    ${MMOTD_TARGET_NAME} SYSTEM
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE ${OPENSSL_INCLUDE_DIR}
    PRIVATE ${JSONCPP_INCLUDE_DIRS}
    PRIVATE ${CMAKE_SOURCE_DIR}/external/certify/include
    )

target_include_directories(
    ${MMOTD_TARGET_NAME} BEFORE
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
