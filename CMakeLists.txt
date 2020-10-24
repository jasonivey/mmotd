# mmotd/CMakeLists.txt
cmake_minimum_required (VERSION 3.10)

project (mmotd-root)

add_subdirectory (lib)
add_subdirectory (app)
add_subdirectory (external/CLI11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
