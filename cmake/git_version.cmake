# cmake/git_version.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.18)

# Grab the version numbers:
include(version)

function (get_mmotd_git_revision OUTPUT_GIT_REVISION)
    set (${OUTPUT_GIT_REVISION} "" PARENT_SCOPE)
    if (EXISTS ${CMAKE_SOURCE_DIR}/.git)
        find_package (Git)
        if (GIT_FOUND)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                ERROR_QUIET
                OUTPUT_VARIABLE "MMOTD_GIT_REVISION"
                OUTPUT_STRIP_TRAILING_WHITESPACE)
            message (STATUS "Mmotd commit: ${MMOTD_GIT_REVISION}")
            set (${OUTPUT_GIT_REVISION} "${MMOTD_GIT_REVISION}" PARENT_SCOPE)
        endif ()
    endif ()
endfunction ()

function (get_mmotd_complete_version MMOTD_VERSION_OUTPUT)
    get_mmotd_version (MAJOR MINOR PATCH)
    string (STRIP ${MAJOR} MAJOR)
    string (STRIP ${MINOR} MINOR)
    string (STRIP ${PATCH} PATCH)
    get_mmotd_git_revision (REVISION)
     if ("x${revision}" STREQUAL "x")
        set (${MMOTD_VERSION_OUTPUT} "${MAJOR}.${MINOR}.${PATCH}" PARENT_SCOPE)
    else ()
        set (${MMOTD_VERSION_OUTPUT} "${MAJOR}.${MINOR}.${PATCH}+${REVISION}" PARENT_SCOPE)
    endif ()
endfunction ()

function (write_version_header)
    set(options DONT_COPY_TO_SOURCE_DIR)
    set(one_value_args)
    set(multi_value_args)
    cmake_parse_arguments(PARSE_ARGV ${ARGC} "VER" "${options}" "${one_value_args}" "${multi_value_args}")
    #cmake_parse_arguments(write_version_header "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    get_mmotd_complete_version(MMOTD_VERSION)
    message (STATUS "Writing out version: ${CMAKE_CURRENT_BINARY_DIR}/common/include/version_number.h")
    configure_file(VERSION.H.IN common/include/version_number.h @ONLY NEWLINE_STYLE UNIX)
    if (${VER_DONT_COPY_TO_SOURCE_DIR})
        message (STATUS "Not writing version: ${CMAKE_CURRENT_SOURCE_DIR}/common/include")
    else ()
        message (STATUS "Writing out version: ${CMAKE_CURRENT_SOURCE_DIR}/common/include")
        file(COPY
            ${CMAKE_CURRENT_BINARY_DIR}/common/include/version_number.h
            DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/common/include
            USE_SOURCE_PERMISSIONS)
    endif ()
endfunction ()

function (should_create_version_headers HEADER_PATH_A HEADER_PATH_B SHOULD_CREATE)
    set (SHOULD_CREATE FALSE)
    if (NOT EXISTS ${HEADER_PATH_A})
        message (STATUS "header does not exist: ${HEADER_PATH_A}")
        message (STATUS "does header exist (don't care): ${HEADER_PATH_B}")
        set (SHOULD_CREATE TRUE)
    else ()
        message (STATUS "header exists: ${HEADER_PATH_A}")
        if (NOT EXISTS ${HEADER_PATH_B})
            message (STATUS "header does not exist: ${HEADER_PATH_B}")
            set (SHOULD_CREATE TRUE)
        else ()
            message (STATUS "header exists: ${HEADER_PATH_B}")
        endif ()
    endif ()
    set(SHOULD_CREATE ${SHOULD_CREATE} PARENT_SCOPE)
endfunction ()

function (are_version_headers_equal HEADER_PATH_A HEADER_PATH_B ARE_EQUAL)
    file(SHA256 ${HEADER_PATH_A} HEADER_A_HASH)
    file(SHA256 ${HEADER_PATH_B} HEADER_B_HASH)
    if (${HEADER_A_HASH} STREQUAL ${HEADER_B_HASH})
        message (STATUS "files identical: '${HEADER_PATH_A}' and '${HEADER_PATH_B}'")
        set(ARE_EQUAL TRUE)
    else ()
        message (STATUS "files are not identical: '${HEADER_PATH_A}' and '${HEADER_PATH_B}'")
        set(ARE_EQUAL FALSE)
    endif ()
    set(ARE_EQUAL ${ARE_EQUAL} PARENT_SCOPE)
endfunction ()

function (update_version_header)
    set (BINARY_VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/common/include/version_number.h)
    set (SOURCE_VERSION_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/common/include/version_number.h)
    set (REWRITE_VERSION_HEADERS TRUE)

    should_create_version_headers (${BINARY_VERSION_HEADER} ${SOURCE_VERSION_HEADER} SHOULD_CREATE)
    if (NOT ${SHOULD_CREATE})
        are_version_headers_equal(${BINARY_VERSION_HEADER} ${SOURCE_VERSION_HEADER} ARE_EQUAL)
        if (${ARE_EQUAL})
            message (STATUS "Existing 'version_number.h' is up-to-date: ${SOURCE_VERSION_HEADER}")
            set (REWRITE_VERSION_HEADERS FALSE)
        endif ()
    endif ()

    if (${REWRITE_VERSION_HEADERS})
        write_version_header ()
    endif ()
endfunction ()

function (if_modified_update_version)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} -C "${CMAKE_CURRENT_SOURCE_DIR}" status --porcelain -uno
        OUTPUT_VARIABLE MMOTD_REPO_GIT_STATUS
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    string(REPLACE "\n" ";" MMOTD_REPO_GIT_STATUS "${MMOTD_REPO_GIT_STATUS}")

    # If cmake/version.cmake is the only modified file
    if ("x${MMOTD_REPO_GIT_STATUS}" STREQUAL "x M cmake/version.cmake")
        get_mmotd_complete_version(MMOTD_VERSION)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} -C "${CMAKE_CURRENT_SOURCE_DIR}" add -u
            )
        execute_process(
            COMMAND ${GIT_EXECUTABLE} -C "${CMAKE_CURRENT_SOURCE_DIR}" commit -m "${MMOTD_VERSION}"
            )
        execute_process(
            COMMAND ${GIT_EXECUTABLE} -C "${CMAKE_CURRENT_SOURCE_DIR}" tag -f -a "${MMOTD_VERSION}" -m "${MMOTD_VERSION}"
            )
        message(STATUS "Version updated to ${MMOTD_VERSION}. Don't forget to push the commit and tag:")
        message(STATUS "  git push origin <feature-branch>")
    endif()
endfunction ()
