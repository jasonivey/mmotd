# cmake/git_version.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.8)

if (EXISTS ${CMAKE_SOURCE_DIR}/.git)
    find_package (Git)
    if (GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE "MMOTD_GIT_REVISION"
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        message (STATUS "Git revision: ${MMOTD_GIT_REVISION}")
    else ()
        set (MMOTD_GIT_REVISION 0)
    endif ()
endif ()

if (EXISTS "${CMAKE_SOURCE_DIR}/VERSION.IN")
    file (READ "${CMAKE_SOURCE_DIR}/VERSION.IN" RAW_MMOTD_VERSION_STR)
else ()
    # FAIL in a big way!
    message (FATAL_ERROR "Critical file not found: ${CMAKE_SOURCE_DIR}/VERSION.IN")
endif ()

string (STRIP ${RAW_MMOTD_VERSION_STR} MMOTD_VERSION_STR)

set (VERSION_SOURCE_FILE_CONTENT "// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
//
// BUILD GENERATED FILE -- DO NOT MODIFY
//   MODIFICATIONS WILL BE OVERWRITTEN
//
namespace mmotd::version {

inline const char *MMOTD_VERSION =\"${MMOTD_VERSION_STR}+${MMOTD_GIT_REVISION}\";

} // namespace mmotd::version
")

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/version_number.h)
    file (READ ${CMAKE_CURRENT_SOURCE_DIR}/include/version_number.h EXISTING_VERSION_SOURCE_FILE_CONTENT)
else ()
    set (EXISTING_VERSION_SOURCE_FILE_CONTENT "")
endif ()

if (NOT "${VERSION_SOURCE_FILE_CONTENT}" STREQUAL "${EXISTING_VERSION_SOURCE_FILE_CONTENT}")
    message (STATUS "Writing out version: ${MMOTD_VERSION_STR}+${MMOTD_GIT_REVISION}")
    file (WRITE ${CMAKE_CURRENT_SOURCE_DIR}/include/version_number.h "${VERSION_SOURCE_FILE_CONTENT}")
endif ()
