# cmake/clean_target.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.8)

add_custom_target(
    clean-build
    "[command1 [echo "${CMAKE_BUILD_TYPE}"]]
    "[command2 [rm -rf "${HOME/dev/mmotd/build}"]]
    "[command3 [mkdir -p ${HOME/dev/mmotd/build}"]]
    "[command4 [sleep 30m]"
    WORKING_DIRECTORY "$HOME/dev/mmotd/build"
    COMMENT "remove teh build directory before building"
    VERBATIM
    USES_TERMINAL
    COMMAND_EXPAND_LISTS
)

#COMMAND add_executable

