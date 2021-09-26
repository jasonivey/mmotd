# cmake/target_common.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.8)

include (set_policies)
set_default_policies()

macro (add_single_definition var_name flag)
    set (existing_flag_value " ${${var_name}} ")
    string (FIND "${existing_flag_value}" " ${flag} " flag_index)
    if (flag_index EQUAL -1)
        # the flag (i.e. -stdlib=libc++) was not found in the variable (i.e. CMAKE_CXX_FLAGS)
        string (COMPARE EQUAL "" "${${var_name}}" is_empty)
        if (is_empty)
            set ("${var_name}" "${flag}")
        else ()
            set ("${var_name}" "${flag} ${${var_name}}")
        endif ()
    endif ()
endmacro ()

if (CMAKE_CXX_COMPILER_ID MATCHES "AppleClang|Clang")
    add_single_definition(CMAKE_CXX_FLAGS "-stdlib=libc++")
    add_single_definition(CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++")
    add_single_definition(CMAKE_EXE_LINKER_FLAGS "-lc++abi")
endif ()

# The Date library is not defining HAS_UNCAUGHT_EXCEPTIONS even though it specifies -std=gnu++17
#  which then results in numerous warnings (not errors so it's just noise) of the type
#  ‘bool std::uncaught_exception()’ is deprecated [-Wdeprecated-declarations]
add_single_definition(CMAKE_CXX_FLAGS "-DHAS_UNCAUGHT_EXCEPTIONS")

include (CheckPIESupported)

include (find_dependencies)

macro (setup_target_properties MMOTD_TARTET_NAME PROJECT_ROOT_INCLUDE_PATH)
    set (CMAKE_CXX_VISIBILITY_PRESET hidden)
    set (CMAKE_VISIBILITY_INLINES_HIDDEN 1)

    set_target_properties(
        ${MMOTD_TARGET_NAME} PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED on
        C_EXTENSIONS off
        CXX_STANDARD 20
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
        # On 'Linux' like compilers this is needed
        PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:_GNU_SOURCE>
        # 'DEBUG', '_DEBUG' and 'NDEBUG' comes for free on MSVC compilers
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:DEBUG>
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:_DEBUG>
        PRIVATE $<$<AND:$<NOT:$<CONFIG:Debug>>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:NDEBUG>
        # This enables the BOOST_ASSERT macro and the "boost::assertion_failed",
        #  "boost::assertion_failed_msg" functions
        PRIVATE BOOST_ENABLE_ASSERT_HANDLER
        # This disables the BOOST_ASSERT macro and the "boost::assertion_failed",
        #  "boost::assertion_failed_msg" functions
        #PRIVATE BOOST_DISABLE_ASSERTS
        # When defined and compiler language is set to `-std=c++17` or higher,
        #  the lambda passed to scope_guard is required to be specified as `noexcept`.
        PRIVATE SG_REQUIRE_NOEXCEPT_IN_CPP17
        # Avoids a compiler warning where the boost::placeholders::_1, _2, _3, etc. end up
        #  conflicting with the ones in the standard C++ library as they are unintendedly
        #  loaded into the global namespace (doh!).
        PRIVATE $<$<CXX_COMPILER_ID:GNU>:BOOST_BIND_GLOBAL_PLACEHOLDERS=1>
        # FMT_ENFORCE_COMPILE_STRING requires all format strings to use FMT_STRING which enables
        #  compile time checking of the format string against the arguments.
        PRIVATE FMT_ENFORCE_COMPILE_STRING
        # When defined the discovery of system properties will be done serially
        #PRIVATE MMOTD_ASYNC_DISABLED
        )

    target_compile_options(
        ${MMOTD_TARGET_NAME}
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
        # Enable C++ dialect options
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:AppleClang,Clang>>:-felide-constructors>
        # Enable various warnings
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Werror>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-pedantic-errors>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wextra>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wformat=2>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wdouble-promotion>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wshadow>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wcast-align>
        PRIVATE $<$<AND:$<NOT:$<CONFIG:Debug>>,$<CXX_COMPILER_ID:AppleClang,Clang,GNU>>:-Wstrict-aliasing>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wfloat-equal>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wwrite-strings>
        # todo: fix_jasoni - these are great warnings but they need a seperate task to enable them
        # PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wconversion>
        # PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wsign-conversion>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wswitch-enum>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wnon-virtual-dtor>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wundef>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wunreachable-code>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wold-style-cast>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Woverloaded-virtual>
        # clang only
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang>:-Wmost>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang>:-Wweak-vtables>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang>:-Wmissing-noreturn>
        # gnu only
        PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wtrampolines>
        PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wlogical-op>
        # disable -- clang
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang>:-Wno-gnu-zero-variadic-macro-arguments>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang>:-Wno-unused-local-typedef>
        # msvc only
        PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/EHsc>                     # ignore asynchronous structured exceptions (SEH)
                                                                     # as SEH does not unwind the stack properly and SE
                                                                     # are almost always FATAL.  If there is a need for
                                                                     # partial recovery in a specific use case then in
                                                                     # that module use '/EHa', __try, __except, and
                                                                     # __finally.
        PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4>                       # increase warning level
        PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/WX>                       # treat all warnings as errors
        PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>                    # set source and execution character sets to be utf-8
        PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>     # set source character sets to be utf-8
        PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/execution-charset:utf-8>  # set execution character sets to be utf-8.

        # /w14242 # 'identfier': conversion from 'type1' to 'type1', possible loss of data
        # /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
        # /w14263 # 'function': member function does not override any base class virtual member function
        # /w14265 # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not be destructed correctly
        # /w14287 # 'operator': unsigned/negative constant mismatch
        # /we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside the for-loop scope
        # /w14296 # 'operator': expression is always 'boolean_value'
        # /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
        # /w14545 # expression before comma evaluates to a function which is missing an argument list
        # /w14546 # function call before comma missing argument list
        # /w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
        # /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
        # /w14555 # expression has no effect; expected expression with side-effect
        # /w14619 # pragma warning: there is no warning number 'number'
        # /w14640 # Enable warning on thread un-safe static member initialization
        # /w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
        # /w14905 # wide string literal cast to 'LPSTR'
        # /w14906 # string literal cast to 'LPWSTR'
        # /w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
        # /w44265
        # /w44265
        # /w44061
        # /w44062
        # /w45038
        )

    # Detect what type of target this is configuring (i.e. STATIC_LIBRARY, SHARED_LIBRARY, EXECUTABLE, etc.)
    get_target_property (target_type ${MMOTD_TARGET_NAME} TYPE)
    string (TOLOWER ${target_type} target_type)

    # All internal include files are relative to the project root.  Include paths relative
    #  to current directory or other schemes are not encouraged.
    target_include_directories(
        ${MMOTD_TARGET_NAME}
        PRIVATE ${PROJECT_ROOT_INCLUDE_PATH}
        )

    target_include_directories(
        ${MMOTD_TARGET_NAME} SYSTEM
        PRIVATE ${Boost_INCLUDE_DIRS}
        PRIVATE ${BACKWARD_INCLUDE_DIRS}
        PRIVATE ${fort_SOURCE_DIR}/lib
        PRIVATE ${certify_SOURCE_DIR}/include
        PRIVATE ${fmt_SOURCE_DIR}/include
        PRIVATE ${random_SOURCE_DIR}/include
        PRIVATE ${json_SOURCE_DIR}/include
        PRIVATE ${date_SOURCE_DIR}/include
        PRIVATE ${scope_guard_SOURCE_DIR}
        PRIVATE ${OPENSSL_INCLUDE_DIR}
        PRIVATE ${toml11_SOURCE_DIR}
        PRIVATE ${cli11_SOURCE_DIR}/include
        PRIVATE $<$<AND:$<STREQUAL:"${target_type}","executable">,$<STREQUAL:"${MMOTD_TARGET_NAME}","mmotd_test">>:${catch2_SOURCE_DIR}/single_include>
        )

    if (target_type STREQUAL "executable")
        target_link_libraries(
            ${MMOTD_TARGET_NAME}
            PRIVATE mmotd_lib
            PRIVATE mmotd_common
            )
        target_link_libraries(
            ${MMOTD_TARGET_NAME}
            PRIVATE fort
            PRIVATE nlohmann_json::nlohmann_json
            PRIVATE fmt::fmt
            PRIVATE date-tz
            PRIVATE OpenSSL::SSL
            PRIVATE OpenSSL::Crypto
            PRIVATE ZLIB::ZLIB
            PRIVATE $<$<CXX_COMPILER_ID:MSVC>:OpenSSL::applink>
            PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang>:-lc++>
            PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang>:-lc++abi>
            PRIVATE $<$<PLATFORM_ID:Darwin>:${FWCoreFoundation}>
            PRIVATE $<$<PLATFORM_ID:Darwin>:${FWSecurity}>
            PRIVATE $<$<STREQUAL:"${MMOTD_TARGET_NAME}","mmotd_test">:Catch2::Catch2>
            PRIVATE Threads::Threads
            PRIVATE ${CMAKE_DL_LIBS}
            )
    endif ()

    # Add the Backward component to the set of includes and linker options
    add_backward (${MMOTD_TARGET_NAME})
endmacro ()
