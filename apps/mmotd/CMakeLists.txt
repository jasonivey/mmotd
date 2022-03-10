# mmotd/app/CMakeLists.txt
cmake_minimum_required (VERSION 3.18)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

set (MMOTD_TARGET_NAME mmotd)

project (mmotd)

add_executable (
    ${MMOTD_TARGET_NAME}
    src/main.cpp
    )

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)

setup_target_properties (${MMOTD_TARGET_NAME} ${PROJECT_ROOT_INCLUDE_PATH})

install(TARGETS "${MMOTD_TARGET_NAME}"
        CONFIGURATIONS "${CMAKE_BUILD_TYPE}"
        RUNTIME DESTINATION "${CMAKE_BUILD_TYPE}/bin")
