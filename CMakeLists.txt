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

# When there is an external API these options will be converted to uses where
#  this project is no longer the root project.  (Possible renaming options to something with 'mmotd')
option(ENABLE_PROFILING "enable code profiling" OFF)
option(ENABLE_COVERAGE "enable code coverage" OFF)
option(ENABLE_SANITIZERS "enable sanitizers" OFF)
option(ENABLE_TESTING "enable building unit-tests" ON)

set_property(GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(target_common)
include(git_version)
update_version_header()
if_modified_update_version()

add_subdirectory(common)
add_subdirectory(lib)
add_subdirectory(apps/mmotd)
add_subdirectory(apps/mmotd_raw)

enable_testing()
add_subdirectory(test)

include(copy_compile_commands)

install(FILES config/mmotd_config.toml config/mmotd_template.json config/softwareengineering.txt
        DESTINATION ".config/mmotd")
