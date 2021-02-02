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

find_package(Threads REQUIRED)
find_package(ZLIB 1.2.11 REQUIRED)
find_package(Boost 1.71.0 REQUIRED)
find_package(OpenSSL 1.1.1 REQUIRED)

include(FetchContent)

# Component: json.  The lightweight repository of nlohmann/json which provides json support.
FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
  GIT_TAG v3.9.1)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()


# Component: certify.  A small module which plugs into boost::beast and boost::asio to support secure https
#            transmissions.  The module handles all of the certificate managment, validation and creation
#            if needed.
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

# Component: fmt.  The library will be added into C++20 and is already a great substitue for both iostreams and
#                  printf.  There is support for most all types, re-ordered argument assignment, and builtin
#                  support for std::date and std::time using the formatting arguments specified with both strftime
#                  and strptime.
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

# Component: PLOG.  A simple logging framework that does just enough of what needs doing.
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

# Component: CLI11.  A simple command line/environment variable/config file parsing component which adds just enough
#                    complexity needed for a project of this scale.
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

# Component: scope_guard.  Adapted and reworked from the originoal article by Peter Petrgu and Andre Alexandru.  I
#                          believe the author was also inspired from the scope guard sources found within the
#                          Facebook folly public repositories.
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

# Component: random.  Wrapper library around the C++11 random facilities.  This library provides all best of the
#                     new C++11 random library without having to 4-5 objects just to generate a random number
#                     between {low, high}.
FetchContent_Declare(
    random
    GIT_REPOSITORY  https://github.com/effolkronium/random.git
    GIT_TAG         v1.3.1
)
FetchContent_GetProperties(random)
if (NOT random_POPULATED)
    FetchContent_Populate(random)
    add_library(random INTERFACE)
    target_include_directories(random INTERFACE ${random_SOURCE_DIR}/include)
endif ()

# Component: range-v3.
FetchContent_Declare(
    ranges
    GIT_REPOSITORY  https://github.com/ericniebler/range-v3
    GIT_TAG         0.11.0
)
FetchContent_GetProperties(ranges)
if (NOT ranges_POPULATED)
    FetchContent_Populate(ranges)
    add_library(ranges INTERFACE)
    target_include_directories(ranges INTERFACE ${ranges_SOURCE_DIR}/include)
endif ()

# Set variable to enable Google Test to update disconnected when available
if (CMAKE_VERSION VERSION_LESS 3.2)
    set (UPDATE_DISCONNECTED_IF_AVAILABLE "")
else ()
    set (UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
endif ()

# Component: googletest.  Using the google test project as our test facility and runner.  Unfortunately, I can't
#                         just use FetchContent on this project.  Instead I'm forced to do something more drastic!
#                         The latest release has to be downloaded, unpacked, configured and built.  All during
#                         configuration time.
include(download-project)
download_project(PROJ                googletest
                 GIT_REPOSITORY      https://github.com/google/googletest.git
                 GIT_TAG             master
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})

add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

