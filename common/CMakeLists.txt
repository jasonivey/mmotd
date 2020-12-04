# mmotd/common/CMakeLists.txt
cmake_minimum_required (VERSION 3.8)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

include (pre_project)

set (MMOTD_TARGET_NAME mmotd_common)

project (mmotd_common)

#add_definitions(-DINSTALL=${CMAKE_BINARY_DIR})
#add_definitions(-DHAS_REMOTE_API=1)
#add_definitions(-DAUTO_DOWNLOAD=1)

add_library (
    ${MMOTD_TARGET_NAME} STATIC
    src/chrono-io.cpp
    src/human_size.cpp
    src/platform_error.cpp
    )

include (target_common)

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)

target_include_directories(
    ${MMOTD_TARGET_NAME}
    PRIVATE ${PROJECT_ROOT_INCLUDE_PATH}
    )

target_include_directories(
    ${MMOTD_TARGET_NAME} SYSTEM
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE ${scope_guard_SOURCE_DIR}
    PRIVATE ${fmt_SOURCE_DIR}/include
    PRIVATE ${plog_SOURCE_DIR}/include
    )

