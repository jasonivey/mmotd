# cmake/target_common.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.8)

include (find_dependencies)
include (add_includes_and_flags)

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

# If we are going to use clang and clang++ then we should also use (but are not forced to) use libc++ instead of stdlibc++.
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    add_cmake_cxx_flags("-stdlib=libc++")
endif ()

message(STATUS "build type: ${CMAKE_BUILD_TYPE}")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DDEBUG)
    add_definitions(-D_DEBUG)
    add_cmake_c_cxx_flags("-Og")
    add_cmake_c_cxx_flags("-g3")
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        add_cmake_c_cxx_flags("-glldb")
        add_cmake_c_cxx_flags("-fdebug-macro")
    else ()
        add_cmake_c_cxx_flags("-ggdb3")
    endif ()
else ()
    add_definitions(-DNDEBUG)
    add_cmake_c_cxx_flags("-O2")
endif ()

# When defined the fmtlib::fmt library will be treated as header-only library
add_definitions(-DFMT_HEADER_ONLY=1)
add_definitions(-DFMT_USE_STRING_VIEW)

# When defined plog will not define generic macro names like:
#  LOG, LOG_ERROR, LOGE, LOG_DEBUG, LOGI
#  These macro names often conflict with other preprocessor macros
#  and cause hard to track down errors.
add_definitions(-DPLOG_OMIT_LOG_DEFINES)

# When defined and compiler language is set to `-std=c++17` or higher,
#  the lambda passed to scope_guard is required to be specified as `noexcept`.
add_definitions(-DSG_REQUIRE_NOEXCEPT_IN_CPP17)

# When defined the discovery of system properties will be done serially
#add_definitions(-DMMOTD_ASYNC_DISABLED)

# Avoids a compiler warning where the boost::placeholders::_1, _2, _3, etc. end up
#  conflicting with the ones in the standard C++ library as they are unintendedly
#  loaded into the global namespace (doh!).
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS=1)
endif ()

# Common compiler options which need to be set for every C++ module
add_cmake_c_cxx_flags(-std=c++17)
add_cmake_c_cxx_flags(-Wall)
add_cmake_c_cxx_flags(-Werror)
add_cmake_c_cxx_flags(-Wpedantic)
add_cmake_c_cxx_flags(-Wextra)
add_cmake_c_cxx_flags(-Wformat=2)
add_cmake_c_cxx_flags(-fPIC)

