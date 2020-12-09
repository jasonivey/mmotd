# cmake/common.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.8)

include (find_dependencies)

# MMOTD_TARGET_NAME needs to be defined in each CMakeLists.txt file
set_target_properties(
    ${MMOTD_TARGET_NAME} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED on
    C_EXTENSIONS off
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED on
    CXX_EXTENSIONS off
    DISABLE_PRECOMPILE_HEADERS on
    )

# If we are going to use clang and clang++ then we should also use
#  (but are not forced to) use libc++ instead of stdlibc++.
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    string(APPEND CMAKE_CXX_FLAGS " -stdlib=libc++")
endif ()

message(STATUS "build type: ${CMAKE_BUILD_TYPE}")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DDEBUG -D_DEBUG)
    string(APPEND CMAKE_CXX_FLAGS " -Og -g3 -ggdb3")
    string(APPEND CMAKE_C_FLAGS " -Og -g3 -ggdb3")
else ()
    add_definitions(-DNDEBUG)
    string(APPEND CMAKE_CXX_FLAGS " -O2")
    string(APPEND CMAKE_C_FLAGS " -O2")
endif ()

add_definitions(-DFMT_HEADER_ONLY=1)
add_definitions(-DFMT_USE_STRING_VIEW)
add_definitions(-DPLOG_OMIT_LOG_DEFINES)
#add_definitions(-DMMOTD_ASYNC_DISABLED=1)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS=1)
endif ()
