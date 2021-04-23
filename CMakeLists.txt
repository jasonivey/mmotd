# mmotd/CMakeLists.txt
cmake_minimum_required(VERSION 3.8)

# update the module path so the include directive finds the module correctly
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(mmotd_root)

get_filename_component(ROOT_CMAKE_PROJECT_DIR . ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR})

define_property(
    GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR INHERITED
    BRIEF_DOCS "mmotd root directory" FULL_DOCS "root directory for the mmotd project"
    )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set_property(GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(target_common)
include(git_version)

add_subdirectory(common)
add_subdirectory(lib)
add_subdirectory(apps/mmotd)
add_subdirectory(apps/mmotd_raw)

enable_testing()
add_subdirectory(test)

include(copy_compile_commands)
