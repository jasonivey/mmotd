# cmake/copy_compile_commands.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.8)

if (CMAKE_EXPORT_COMPILE_COMMANDS AND EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")
    message (STATUS "Updating compile_commands.json")
    EXECUTE_PROCESS (COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
        )
endif ()
