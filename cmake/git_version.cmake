# cmake/git_version.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.8)

# Grab the version numbers:
include(version)

set(MMOTD_MAJOR_VERSION ${MMOTD_MAJOR})
string (STRIP ${MMOTD_MAJOR_VERSION} MMOTD_MAJOR_VERSION)
set(MMOTD_MINOR_VERSION ${MMOTD_MINOR})
string (STRIP ${MMOTD_MINOR_VERSION} MMOTD_MINOR_VERSION)
set(MMOTD_PATCHLEVEL_VERSION ${MMOTD_PATCHLEVEL})
string (STRIP ${MMOTD_PATCHLEVEL_VERSION} MMOTD_PATCHLEVEL_VERSION)
set(MMOTD_VERSION ${MMOTD_MAJOR_VERSION}.${MMOTD_MINOR_VERSION}.${MMOTD_PATCHLEVEL_VERSION})

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

message (STATUS "writing out version: common/include/version_number.h")
configure_file(VERSION.H.IN common/include/version_number.h @ONLY)
file(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/common/include/version_number.h MMOTD_OLD_VERSION_H)
file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/common/include/version_number.h MMOTD_NEW_VERSION_H)

if (NOT MMOTD_NEW_VERSION_H STREQUAL MMOTD_OLD_VERSION_H)
    set(MMOTD_GIT_STATUS "")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} -C "${CMAKE_CURRENT_SOURCE_DIR}" status --porcelain -uno
        OUTPUT_VARIABLE MMOTD_GIT_STATUS
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    string(REPLACE "\n" ";" MMOTD_GIT_STATUS ${MMOTD_GIT_STATUS})

    # If cmake/version.cmake is the only modified file
    if (NOT "x${MMOTD_GIT_STATUS}" STREQUAL "x M cmake/version.cmake")
        message(STATUS "not creating tag for 'cmake/version.cmake'")
    else ()
        execute_process(
            COMMAND ${GIT_EXECUTABLE} -C "${CMAKE_CURRENT_SOURCE_DIR}" add -u
            )
        execute_process(
            COMMAND ${GIT_EXECUTABLE} -C "${CMAKE_CURRENT_SOURCE_DIR}" commit -m "${MMOTD_VERSION}"
            )
        execute_process(
            COMMAND ${GIT_EXECUTABLE} -C "${CMAKE_CURRENT_SOURCE_DIR}" tag -f -a "${MMOTD_VERSION}" -m "${MMOTD_VERSION}"
            )
        message(STATUS "Version updated to ${MMOTD_VERSION}. Don't forget to:")
        message(STATUS "  git push origin <feature-branch>")
    endif()
endif()

