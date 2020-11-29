# cmake/find_dependencies.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.8)

set (CMAKE_WARN_DEPRECATED FALSE CACHE BOOL "Whether to issue warnings for deprecated functionality." FORCE)

if (APPLE)
    # enables setting ${package}_root variables like ZLIB_ROOT without warning
    if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12")
        cmake_policy(SET CMP0074 NEW)
    endif ()
    set (ZLIB_ROOT /usr/local/opt/zlib)
endif ()

find_package(ZLIB 1.2.11 REQUIRED)
find_package(Boost 1.74.0 REQUIRED)
find_package(OpenSSL 1.1.1 REQUIRED)

include(FetchContent)

FetchContent_Declare(
    certify
    GIT_REPOSITORY https://github.com/djarek/certify.git
)

FetchContent_GetProperties(certify)
if (NOT certify_POPULATED)
    FetchContent_Populate(certify)
    add_library(certify INTERFACE)
    target_include_directories(certify INTERFACE ${certify_SOURCE_DIR}/include)
endif ()

FetchContent_Declare(
    fmt
    GIT_REPOSITORY  https://github.com/fmtlib/fmt.git
    GIT_TAG         7.1.3
)

FetchContent_GetProperties(fmt)
if (NOT fmt_POPULATED)
    FetchContent_Populate(fmt)
    add_library(fmt INTERFACE)
    target_include_directories(fmt INTERFACE ${fmt_SOURCE_DIR}/include)
endif ()

FetchContent_Declare(
    plog
    GIT_REPOSITORY  https://github.com/SergiusTheBest/plog.git
    GIT_TAG         1.1.5
)

FetchContent_GetProperties(plog)
if (NOT plog_POPULATED)
    FetchContent_Populate(plog)
    add_library(plog INTERFACE)
    target_include_directories(plog INTERFACE ${plog_SOURCE_DIR}/include)
endif ()

FetchContent_Declare(
    cli11
    GIT_REPOSITORY  https://github.com/CLIUtils/CLI11.git
    # The git tag of 'v1.9.1' is actually valid.  But I started using HEAD and now I am reliant
    #  on a few of the features there.  Once CLI11 creates a new release I will update this
    #  rely on that tag
    #GIT_TAG         v1.9.1
)

FetchContent_GetProperties(cli11)
if (NOT cli11_POPULATED)
    FetchContent_Populate(cli11)
    add_library(cli11 INTERFACE)
    target_include_directories(cli11 INTERFACE ${cli11_SOURCE_DIR}/include)
endif ()

FetchContent_Declare(
    scope_guard
    GIT_REPOSITORY  https://github.com/ricab/scope_guard.git
    GIT_TAG         v0.2.3
)

FetchContent_GetProperties(scope_guard)
if (NOT scope_guard_POPULATED)
    FetchContent_Populate(scope_guard)
    add_library(scope_guard INTERFACE)
    target_include_directories(scope_guard INTERFACE ${scope_guard_SOURCE_DIR})
endif ()
