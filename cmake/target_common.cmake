# cmake/target_common.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.8)

# CMake 3.13 and lower did not add `PIE` or `PIC` link flags when POSITION_INDEPENDENT_CODE is set
cmake_policy(SET CMP0083 NEW)
include (CheckPIESupported)

include (find_dependencies)

macro (setup_target_properties MMOTD_TARTET_NAME PROJECT_ROOT_INCLUDE_PATH)
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

    # PIE is set to FALSE because boost stack trace forces it.
    # TODO: when creating actual shared-object enable PIC in release mode
    #add_definitions(-fPIC)
    set_property(TARGET ${MMOTD_TARGET_NAME} PROPERTY POSITION_INDEPENDENT_CODE FALSE)

    if (APPLE)
        find_library(FWCoreFoundation NAMES CoreFoundation REQUIRED)
        find_library(FWSecurity NAMES Security REQUIRED)
    endif ()

    target_compile_definitions(
        ${MMOTD_TARGET_NAME}
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:_GNU_SOURCE>
        PRIVATE $<$<CONFIG:Debug>:_DEBUG>
        PRIVATE $<$<CONFIG:Debug>:DEBUG>
        PRIVATE $<$<CONFIG:Release>:NDEBUG>
        # This enables the BOOST_ASSERT macro and the "boost::assertion_failed",
        #  "boost::assertion_failed_msg" functions
        PRIVATE BOOST_ENABLE_ASSERT_HANDLER
        # This disables the BOOST_ASSERT macro and the "boost::assertion_failed",
        #  "boost::assertion_failed_msg" functions
        #PRIVATE BOOST_DISABLE_ASSERTS
        # When defined plog will not define generic macro names like:
        #  LOG, LOG_ERROR, LOGE, LOG_DEBUG, LOGI
        #  These macro names often conflict with other preprocessor macros
        #  and cause hard to track down errors.
        PRIVATE PLOG_OMIT_LOG_DEFINES
        # When defined and compiler language is set to `-std=c++17` or higher,
        #  the lambda passed to scope_guard is required to be specified as `noexcept`.
        PRIVATE SG_REQUIRE_NOEXCEPT_IN_CPP17
        # Avoids a compiler warning where the boost::placeholders::_1, _2, _3, etc. end up
        #  conflicting with the ones in the standard C++ library as they are unintendedly
        #  loaded into the global namespace (doh!).
        PRIVATE $<$<CXX_COMPILER_ID:GNU>:BOOST_BIND_GLOBAL_PLACEHOLDERS=1>
        # When defined the discovery of system properties will be done serially
        #PRIVATE MMOTD_ASYNC_DISABLED
        )

    target_compile_options(
        ${MMOTD_TARGET_NAME}
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Werror>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wpedantic>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wextra>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wformat=2>
        # If we are going to use clang and clang++ then we should also use,
        #  (but are not forced to) use libc++ instead of stdlibc++.
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang>:-stdlib=libc++>
        # Enable debug information for stack tracing purposes on all build types
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-g>
        # Enable various debug build options on debug builds
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:AppleClang,Clang,GNU>>:-Og>
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:AppleClang,Clang,GNU>>:-g3>
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:AppleClang,Clang>>:-glldb>
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:AppleClang,Clang>>:-fdebug-macro>
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:-ggdb3>
        # Enable optimizations on release builds
        PRIVATE $<$<AND:$<NOT:$<CONFIG:Debug>>,$<CXX_COMPILER_ID:AppleClang,Clang,GNU>>:-O2>
        )

    # This adds the root mmotd source directory as the only project include directory.
    #  This means all include files will be relative to this project root directory.
    #  For example:
    #    #include "common/include/algorithm.h"
    #    #include "apps/mmotd/include/cli_app_options_creator.h"
    target_include_directories(
        ${MMOTD_TARGET_NAME}
        PRIVATE ${PROJECT_ROOT_INCLUDE_PATH}
        )

    target_include_directories(
        ${MMOTD_TARGET_NAME} SYSTEM
        PRIVATE ${Boost_INCLUDE_DIRS}
        PRIVATE ${certify_SOURCE_DIR}/include
        PRIVATE ${plog_SOURCE_DIR}/include
        PRIVATE ${fmt_SOURCE_DIR}/include
        PRIVATE ${random_SOURCE_DIR}/include
        PRIVATE ${json_SOURCE_DIR}/include
        PRIVATE ${scope_guard_SOURCE_DIR}
        PRIVATE ${OPENSSL_INCLUDE_DIR}
        )

    get_target_property (target_type ${MMOTD_TARGET_NAME} TYPE)
    string (TOLOWER ${target_type} target_type)
    if (target_type STREQUAL "executable")
        if (${MMOTD_TARGET_NAME} STREQUAL "mmotd_test")
            target_include_directories(
                ${MMOTD_TARGET_NAME} SYSTEM
                PRIVATE ${catch2_SOURCE_DIR}/single_include
                )
        else ()
            target_include_directories(
                ${MMOTD_TARGET_NAME} SYSTEM
                PRIVATE ${cli11_SOURCE_DIR}/include
                )
        endif ()
        target_link_libraries(
            ${MMOTD_TARGET_NAME}
            PRIVATE mmotd_common
            PRIVATE mmotd_lib
            )
        target_link_libraries(
            ${MMOTD_TARGET_NAME}
            PRIVATE nlohmann_json::nlohmann_json
            PRIVATE $<$<PLATFORM_ID:Darwin>:${FWCoreFoundation}>
            PRIVATE $<$<PLATFORM_ID:Darwin>:${FWSecurity}>
            PRIVATE $<$<PLATFORM_ID:Darwin,Linux>:${OPENSSL_CRYPTO_LIBRARY}>
            PRIVATE $<$<PLATFORM_ID:Darwin,Linux>:${OPENSSL_SSL_LIBRARY}>
            PRIVATE $<$<PLATFORM_ID:Darwin,Linux>:ZLIB::ZLIB>
            PRIVATE $<$<STREQUAL:target_type,"mmotd_test">:Catch2::Catch2>
            PRIVATE Threads::Threads
            )
    endif ()

    # Add the Backward component to the set of includes and linker options
    add_backward (${MMOTD_TARGET_NAME})
endmacro ()

