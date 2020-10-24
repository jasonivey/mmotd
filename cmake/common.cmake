# cmake/common.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.10)

if (APPLE)
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
endif ()

add_compile_options(-Wall -Werror -Wpedantic -Wextra -Wformat=2 -fPIC)

# MMOTD_TARGET_NAME needs to be defined in each CMakeLists.txt file
set_target_properties(
    ${MMOTD_TARGET_NAME} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED on
    C_EXTENSIONS off
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED on
    CXX_EXTENSIONS off
    )

find_package(Boost COMPONENTS log_setup log REQUIRED)
find_package(fmt REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

find_package(Git)
if (GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if (GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if (NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with '${GIT_SUBMOD_RESULT}', please checkout submodules")
        endif ()
    endif ()
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    string(APPEND CMAKE_CXX_FLAGS " -stdlib=libc++")
endif ()

message("build type: ${CMAKE_BUILD_TYPE}")

add_definitions(-DBOOST_LOG_DYN_LINK)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DDEBUG -D_DEBUG)
    string(APPEND CMAKE_CXX_FLAGS " -Og -g3 -ggdb3")
    string(APPEND CMAKE_C_FLAGS " -Og -g3 -ggdb3")
else ()
    string(APPEND CMAKE_CXX_FLAGS " -O2")
    string(APPEND CMAKE_C_FLAGS " -O2")
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
