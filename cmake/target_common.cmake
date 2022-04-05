# cmake/target_common.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.18)

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

# If it isn't placed in the GLOBAL cxx and link flags the libc++ standard
#  library won't be used in 3rd party code. This leads to link errors or
#  worse, really confusing runtime behavior.
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
        # On non-Windows C++ standard libraraies _GNU_SOURCE definition is required.
        PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:_GNU_SOURCE>
        PRIVATE $<$<CXX_COMPILER_ID:GNU>:_GLIBCXX_USE_CXX11_ABI=1>
        # 'DEBUG', '_DEBUG' and 'NDEBUG' comes for free on MSVC compilers
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:DEBUG>
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:_DEBUG>
        PRIVATE $<$<AND:$<NOT:$<CONFIG:Debug>>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:NDEBUG>
        # Windows specific options (strip platform headers to the bare minimum)
        PRIVATE $<$<CXX_COMPILER_ID:MSVC>:WIN32_LEAN_AND_MEAN>
        # If 'CATCH_CONFIG_PREFIX_ALL' is defined then all CATCH unit test macros are
        #  prefixed with CATCH_. This is necessary since the MMOTD project defines
        #  it's own version of 'CHECK' as a design by contract specifier.
        PRIVATE CATCH_CONFIG_PREFIX_ALL
        # When defined and compiler language is set to `-std=c++17` or higher,
        #  the lambda passed to scope_guard is required to be specified as `noexcept`.
        PRIVATE SG_REQUIRE_NOEXCEPT_IN_CPP17
        # FMT_ENFORCE_COMPILE_STRING requires all format strings to use FMT_STRING which enables
        #  compile time checking of the format string against the arguments.
        PRIVATE FMT_ENFORCE_COMPILE_STRING
        # Boost ASIO has not updated it's code for C++20 and the removal of `std::result_of`
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang>:BOOST_ASIO_HAS_STD_INVOKE_RESULT>
        # By default, enable preserving comments within the toml library
        PRIVATE TOML11_PRESERVE_COMMENTS_BY_DEFAULT
        # When defined the discovery of system properties will be done serially
        #PRIVATE MMOTD_ASYNC_DISABLED
        )

    target_compile_options(
        ${MMOTD_TARGET_NAME}
        # If we are going to use clang and clang++ then we should also,
        #  but are not forced to, use libc++ instead of stdlibc++.
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang>:-stdlib=libc++>

        # PROFILING OPTIONS...
        PRIVATE $<$<BOOL:${ENABLE_PROFILING}>:-pg>

        # COVERAGE OPTIONS...
        PRIVATE $<$<BOOL:${ENABLE_COVERAGE}>:--coverage>
        PRIVATE $<$<BOOL:${ENABLE_COVERAGE}>:-ftest-coverage>
        PRIVATE $<$<BOOL:${ENABLE_COVERAGE}>:-fprofile-arcs>

        # DEBUGGING & DEBUG INFO...
        # On RELEASE builds embed debug information for stack trace information
        PRIVATE $<$<AND:$<NOT:$<CONFIG:Debug>>,$<CXX_COMPILER_ID:AppleClang,Clang,GNU>>:-g>
        # On DEBUG builds set '-g3' which enables features to ensure a quality debugging experience
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:AppleClang,Clang,GNU>>:-g3>
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:AppleClang,Clang>>:-glldb>
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:-ggdb>
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:AppleClang,Clang>>:-fdebug-macro>
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:AppleClang,Clang>>:-gcolumn-info>
        #PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:AppleClang,Clang>>:-grecord-command-line>

        # OPTIMIZATIONS...
        # Reduce compilation time and make debugging produce the expected results. This is the default.
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:AppleClang,Clang,GNU>>:-O0>
        # Enable optimizations (speed & size) on release builds
        PRIVATE $<$<AND:$<NOT:$<CONFIG:Debug>>,$<CXX_COMPILER_ID:AppleClang,Clang,GNU>>:-O2>
        # Enable C++ dialect options
        PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:AppleClang,Clang>>:-felide-constructors>
        # Enable char8_t on clang compilers
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang>:-fchar8_t>
        # Enable colorized output to always
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang>:-fcolor-diagnostics>
        PRIVATE $<$<CXX_COMPILER_ID:GNU>:-fdiagnostics-color=always>
        # Enable warnings as errors
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Werror>
        # Give an error whenever the standard (i.e. -std=c++20) requires a diagnostic, in some cases where there is
        #  undefined behavior at compile-time and in some other cases that do not prevent compilation of programs that
        #  are valid according to the standard.
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-pedantic-errors>
        # enables all the warnings about constructions that are questionable
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall>
        # enables some extra warning flags that are not enabled by -Wall
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wextra>
        # warn the user if a variable declaration shadows one from a parent context
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wshadow>
        # warn the user if a class with virtual functions has a non-virtual destructor
        #  this helps catch hard to track down memory errors
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wnon-virtual-dtor>
        # helps to identify those hidden c-style casts
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wold-style-cast>
        # warn whenever a pointer is cast such that the required alignment of the target is increased. for example,
        #  warn if a `char *` is cast to an `int *` on machines where integers can only be accessed at two- or
        #  four-byte boundaries.
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wcast-align>
        # same as above except change 'on machines where integers...' to 'regardless of the target machine' (strict)
        PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wcast-align=strict>
        # warn on anything being unused
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wunused>
        # warn if you overload (not override) a virtual function
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Woverloaded-virtual>
        # warn on type conversions that may lose data
        # TODO: fix_jasoni... re-enable
        PRIVATE $<$<AND:$<BOOL:FALSE>,$<CXX_COMPILER_ID:AppleClang,Clang,GNU>>:-Wconversion>
        # warn on sign conversions
        # TODO: fix_jasoni... re-enable
        PRIVATE $<$<AND:$<BOOL:FALSE>,$<CXX_COMPILER_ID:AppleClang,Clang,GNU>>:-Wsign-conversion>
        # warn if float is implicit promoted to double
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wdouble-promotion>
        # warn on security issues around functions that format output (ie `printf`)
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wformat=2>
        PRIVATE $<$<AND:$<NOT:$<CONFIG:Debug>>,$<CXX_COMPILER_ID:AppleClang,Clang,GNU>>:-Wstrict-aliasing>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wfloat-equal>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wwrite-strings>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wswitch-enum>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wundef>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wunused-macros>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wunreachable-code>

        # LLVM clang only
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang>:-Wmost>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang>:-Wweak-vtables>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang>:-Wmissing-noreturn>
        PRIVATE $<$<CXX_COMPILER_ID:AppleClang,Clang>:-Wdtor-name>

        # GNU g++ only
        # warn if indentation implies blocks where blocks do not exist
        PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wmisleading-indentation>
        # warn if `if / else` chain has duplicated conditions
        PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wduplicated-cond>
        # warn if `if / else` branches have duplicated code
        PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wduplicated-branches>
        # warn about wrapping a pointer or data in a stack object to call it indirectly (i.e. marking the stack executable)
        PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wtrampolines>
        # warn about logical operations being used where bitwise were probably wanted
        PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wlogical-op>
        # warn if a null dereference is detected
        PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wnull-dereference>
        # warn if you perform a cast to the same type
        # TODO: fix_jasoni... re-enable
        PRIVATE $<$<AND:$<BOOL:FALSE>,$<CXX_COMPILER_ID:GNU>>:-Wuseless-cast>
        # TODO: fix_jasoni... re-enable
        PRIVATE $<$<AND:$<BOOL:FALSE>,$<CXX_COMPILER_ID:AppleClang,Clang,GNU>>:-Wcast-qual>

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
        PRIVATE ${BACKWARD_INCLUDE_DIRS}
        PRIVATE ${Boost_INCLUDE_DIRS}
        # PRIVATE ${certify_SOURCE_DIR}/include
        PRIVATE ${CURL_INCLUDE_DIR}
        PRIVATE ${cli11_SOURCE_DIR}/include
        PRIVATE ${date_SOURCE_DIR}/include
        PRIVATE ${fmt_SOURCE_DIR}/include
        PRIVATE ${json_SOURCE_DIR}/single_include
        PRIVATE ${OPENSSL_INCLUDE_DIR}
        PRIVATE ${random_SOURCE_DIR}/include
        PRIVATE ${scope_guard_SOURCE_DIR}
        PRIVATE ${toml11_SOURCE_DIR}
        PRIVATE ${utfcpp_SOURCE_DIR}/source
        PRIVATE $<$<AND:$<STREQUAL:"${target_type}","executable">,$<STREQUAL:"${MMOTD_TARGET_NAME}","mmotd_test">>:${catch2_SOURCE_DIR}/single_include>
        )

    if (target_type STREQUAL "executable")
        target_link_options(
            ${MMOTD_TARGET_NAME}
            # PROFILING OPTIONS...
            PRIVATE $<$<BOOL:${ENABLE_PROFILING}>:-pg>
            # COVERAGE OPTIONS...
            PRIVATE $<$<BOOL:${ENABLE_COVERAGE}>:--coverage>
            PRIVATE $<$<BOOL:${ENABLE_COVERAGE}>:-fprofile-arcs>
            PRIVATE $<$<BOOL:${ENABLE_COVERAGE}>:-ftest-coverage>
            # SWITCH DEFAULT LINKER: valid values:
            #  - bfd  : The 'Binary File Descriptor' linker
            #  - gold : The GNU ELF linker
            #  - lld  : The LLVM linker
            #  - mold : The modern linker
            #PRIVATE $<$<CXX_COMPILER_ID:Clang>:-fuse-ld=lld>
            )
        target_link_libraries(
            ${MMOTD_TARGET_NAME}
            PRIVATE mmotd_lib
            PRIVATE mmotd_common
            )
        target_link_libraries(
            ${MMOTD_TARGET_NAME}
            INTERFACE utfcpp
            PRIVATE ${CURL_LIBRARIES}
            INTERFACE nlohmann_json::nlohmann_json
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
            PRIVATE $<$<PLATFORM_ID:Darwin>:${FWIOKit}>
            PRIVATE $<$<STREQUAL:"${MMOTD_TARGET_NAME}","mmotd_test">:Catch2::Catch2>
            PRIVATE Threads::Threads
            PRIVATE ${CMAKE_DL_LIBS}
            )
    endif ()

    # Add the Backward component to the set of includes and linker options
    add_backward (${MMOTD_TARGET_NAME})
endmacro ()
