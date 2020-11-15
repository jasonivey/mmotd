# mmotd/CMakeLists.txt
cmake_minimum_required (VERSION 3.18)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include (pre_project)

project (mmotd_root)

define_property(
    GLOBAL
    PROPERTY
    ROOT_CMAKE_PROJECT_DIR
    INHERITED
    BRIEF_DOCS "x"
    FULL_DOCS "yy"
    )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set_property(GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include (common)

add_subdirectory (view)
add_subdirectory (lib)
add_subdirectory (app)
