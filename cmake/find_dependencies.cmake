# cmake/find_dependencies.cmake
include_guard (DIRECTORY)

Include(message_quiet)

cmake_minimum_required (VERSION 3.8)

set (CMAKE_WARN_DEPRECATED FALSE CACHE BOOL "Whether to issue warnings for deprecated functionality." FORCE)

include (set_policies)
set_default_policies()

if (APPLE)
    set (ZLIB_ROOT /usr/local/opt/zlib)
endif ()

find_package(Threads REQUIRED)
find_package(ZLIB 1.2.11 REQUIRED)
find_package(Boost 1.71.0 REQUIRED)
find_package(OpenSSL 1.1.1 REQUIRED)

Include(FetchContent)

if (APPLE)
    # Find various include and framework directories on macOS
    find_path(LIBBFD_INCLUDE_DIR NAMES "bfd.h" PATHS /usr/local/opt/binutils/include)
    find_path(LIBDL_INCLUDE_DIR NAMES "dlfcn.h" PATHS ${CMAKE_OSX_SYSROOT}/usr/include)
    find_library(LIBBFD_LIBRARY bfd PATHS /usr/local/opt/binutils/lib)
    find_library(LIBIBERTY_LIBRARY iberty PATHS /usr/local/opt/binutils/lib)
    find_library(FWCoreFoundation NAMES CoreFoundation REQUIRED)
    find_library(FWSecurity NAMES Security REQUIRED)
endif ()

# Component: date.  Howard Hinnant's Date library (author of the std::chrono library among others)
option (BUILD_TZ_LIB "build/install of TZ library" ON)
FetchContent_Declare(date
    GIT_REPOSITORY   https://github.com/HowardHinnant/date.git
    GIT_TAG          v3.0.0
    GIT_PROGRESS     TRUE
)
set(MESSAGE_QUIET ON)
FetchContent_MakeAvailable(date)
unset(MESSAGE_QUIET)

# Component: Backward.  Stack trace library
FetchContent_Declare(Backward
    GIT_REPOSITORY   https://github.com/bombela/backward-cpp.git
    #GIT_TAG          v1.5
    #GIT_PROGRESS     TRUE
)
set(MESSAGE_QUIET ON)
FetchContent_MakeAvailable(Backward)
unset(MESSAGE_QUIET)

# Component: libfort (Library to create FORmatted Tables)
set(FORT_ENABLE_TESTING OFF CACHE INTERNAL "")
FetchContent_Declare(fort
    GIT_REPOSITORY   https://github.com/seleznevae/libfort.git
    GIT_TAG          v0.4.2
    GIT_PROGRESS     TRUE
)
FetchContent_GetProperties(fort)
if (NOT fort_POPULATED)
    FetchContent_Populate(fort)
    set(MESSAGE_QUIET ON)
    add_subdirectory(${fort_SOURCE_DIR} ${fort_BINARY_DIR} EXCLUDE_FROM_ALL)
    unset(MESSAGE_QUIET)
endif ()

# Component: Catch2.  Unit testing framework
FetchContent_Declare(catch2
    GIT_REPOSITORY   https://github.com/catchorg/Catch2.git
    GIT_TAG          v2.13.4
    GIT_PROGRESS     TRUE
)
FetchContent_MakeAvailable(catch2)

# Component: json.  The lightweight repository of nlohmann/json which provides json support.
FetchContent_Declare(json
    GIT_REPOSITORY   https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
    GIT_TAG          v3.9.1
    GIT_PROGRESS     TRUE
)
FetchContent_GetProperties(json)
if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Component: certify.  A small module which plugs into boost::beast and boost::asio to support secure https
#            transmissions.  The module handles all of the certificate managment, validation and creation
#            if needed.
FetchContent_Declare(certify
    GIT_REPOSITORY   https://github.com/djarek/certify.git
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
FetchContent_Declare(fmt
    GIT_REPOSITORY   https://github.com/fmtlib/fmt.git
    GIT_TAG          8.0.0
    GIT_PROGRESS     TRUE
)
FetchContent_GetProperties(fmt)
if (NOT fmt_POPULATED)
    FetchContent_Populate(fmt)
    set(MESSAGE_QUIET ON)
    add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR} EXCLUDE_FROM_ALL)
    unset(MESSAGE_QUIET)
endif ()

# Component: CLI11.  A simple command line/environment variable/config file parsing component which adds just enough
#                    complexity needed for a project of this scale.
FetchContent_Declare(cli11
    GIT_REPOSITORY   https://github.com/CLIUtils/CLI11.git
    # The git tag of 'v1.9.1' is actually valid.  But I started using HEAD and now I am reliant
    #  on a few of the features there.  Once CLI11 creates a new release I will update this
    #  rely on that tag
    #GIT_TAG         v1.9.1
    GIT_PROGRESS     TRUE
)
FetchContent_GetProperties(cli11)
if (NOT cli11_POPULATED)
    FetchContent_Populate(cli11)
    add_library(cli11 INTERFACE)
    target_include_directories(cli11 INTERFACE ${cli11_SOURCE_DIR}/include)
endif ()

# Component: scope_guard.  Adapted and reworked from the original article by Peter Petrgu and Andre Alexandru.
#                          I believe the author was also inspired from the scope guard sources found within the
#                          Facebook folly public repositories:
#                           https://raw.githubusercontent.com/facebook/folly/master/folly/ScopeGuard.h
#                          But the author's work is much more concise and simple in my opinion.
FetchContent_Declare(scope_guard
    GIT_REPOSITORY   https://github.com/ricab/scope_guard.git
    GIT_TAG          v0.2.3
    GIT_PROGRESS     TRUE
)
FetchContent_GetProperties(scope_guard)
if (NOT scope_guard_POPULATED)
    FetchContent_Populate(scope_guard)
    add_library(scope_guard INTERFACE)
    target_include_directories(scope_guard INTERFACE ${scope_guard_SOURCE_DIR})
endif ()

# Component: random.  Wrapper library around the standard C++11 random number facilities.  This library
#                     provides simple API's to generate random numbers.  It's used within this project for
#                     selecting a random element in a given container.  The upside to using a wrapper
#                     library for the random number facilities is huge for C++.  There isn't a need to worry
#                     about the various objects which need to be choosen and created such as:
#                      1. engines
#                      2. generators
#                      3. devices (i.e deterministic or non-deterministic)
#                      4. distributions (uniform, normal, sampling, bernoulli, ?, etc.)
#                      5. ... and the non-trivial concept of seeding (i.e when, how, how often, etc.)
FetchContent_Declare(random
    GIT_REPOSITORY   https://github.com/effolkronium/random.git
    GIT_TAG          v1.3.1
    GIT_PROGRESS     TRUE
)
FetchContent_GetProperties(random)
if (NOT random_POPULATED)
    FetchContent_Populate(random)
    add_library(random INTERFACE)
    target_include_directories(random INTERFACE ${random_SOURCE_DIR}/include)
endif ()
