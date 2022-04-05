# cmake/set_policies.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.18)

macro (set_default_policies)
    # Included scripts do automatic cmake_policy PUSH and POP.
    if (POLICY CMP0011)
        cmake_policy(SET CMP0011 NEW)
    endif ()

    # Honor visibility properties for all target types
    if (POLICY CMP0063)
        cmake_policy (SET CMP0063 NEW)
    endif ()

    # find_package() uses <PackageName>_ROOT variables.
    if (POLICY CMP0074)
        cmake_policy(SET CMP0074 NEW)
    endif ()

    # To control generation of Position Independent Executable (PIE) or not, some flags are required at link time.
    if (POLICY CMP0083)
        cmake_policy(SET CMP0083 NEW)
    endif ()

    # add_test() supports arbitrary characters in test names.
    if (POLICY CMP0110)
        cmake_policy(SET CMP0110 NEW)
    endif ()
endmacro ()
