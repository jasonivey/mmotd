# cmake/common.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.10)

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

# TODO: Add better-enums: https://github.com/aantron/better-enums

# TODO: Create install and doc targets.  Use `standardese` as documentation generator for installed headers
# INSTALL: brew install standardese
#find_package(standardese REQUIRED) # find standardese after installation

# generates a custom target that will run standardese to generate the documentation
#standardese_generate(my_target CONFIG path/to/config_file
#                     INCLUDE_DIRECTORY ${my_target_includedirs}
#                     INPUT ${headers})
