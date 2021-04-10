# mmotd/lib/CMakeLists.txt
cmake_minimum_required (VERSION 3.8)

# update the module path so the include directive finds the module correctly
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

include (pre_project)

set (MMOTD_TARGET_NAME mmotd_lib)

project (mmotd_lib)

set (MMOTD_LIB_SOURCES
    src/boot_time.cpp
    src/computer_information.cpp
    src/external_network.cpp
    src/file_system.cpp
    src/fortune.cpp
    src/general.cpp
    src/http_request.cpp
    src/information_provider.cpp
    src/lastlog.cpp
    src/load_average.cpp
    src/memory.cpp
    src/network.cpp
    src/platform/system_information.cpp
    src/processes.cpp
    src/swap.cpp
    src/system_details.cpp
    src/system_information.cpp
    src/users_logged_in.cpp
    src/weather_info.cpp)


if (APPLE)
    list(APPEND MMOTD_LIB_SOURCES
        src/platform/darwin/boot_time.cpp
        src/platform/darwin/lastlog.cpp
        src/platform/darwin/load_average.cpp
        src/platform/darwin/memory.cpp
        src/platform/darwin/network.cpp
        src/platform/darwin/processes.cpp
        src/platform/darwin/swap.cpp
        src/platform/darwin/system_information.cpp
        src/platform/darwin/user_accounting_database.cpp)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    list(APPEND MMOTD_LIB_SOURCES
        src/platform/linux/boot_time.cpp
        src/platform/linux/lastlog.cpp
        src/platform/linux/load_average.cpp
        src/platform/linux/memory.cpp
        src/platform/linux/network.cpp
        src/platform/linux/processes.cpp
        src/platform/linux/swap.cpp
        src/platform/linux/system_information.cpp
        src/platform/linux/user_accounting_database.cpp)
elseif (WIN32)
    list(APPEND MMOTD_LIB_SOURCES
        src/platform/windows/boot_time.cpp
        src/platform/windows/lastlog.cpp
        src/platform/windows/load_average.cpp
        src/platform/windows/memory.cpp
        src/platform/windows/network.cpp
        src/platform/windows/processes.cpp
        src/platform/windows/swap.cpp
        src/platform/windows/system_information.cpp
        src/platform/windows/user_accounting_database.cpp)
endif ()

add_library (${MMOTD_TARGET_NAME} STATIC ${MMOTD_LIB_SOURCES})

setup_target_properties (${MMOTD_TARGET_NAME})

get_property(PROJECT_ROOT_INCLUDE_PATH GLOBAL PROPERTY ROOT_CMAKE_PROJECT_DIR)

add_cmake_c_cxx_include_directory(${PROJECT_ROOT_INCLUDE_PATH} FALSE)
add_cmake_c_cxx_include_directory(${OPENSSL_INCLUDE_DIR} TRUE)
add_cmake_c_cxx_include_directory(${json_SOURCE_DIR}/include TRUE)
add_cmake_c_cxx_include_directory(${Boost_INCLUDE_DIRS} TRUE)
add_cmake_c_cxx_include_directory(${scope_guard_SOURCE_DIR} TRUE)
add_cmake_c_cxx_include_directory(${certify_SOURCE_DIR}/include TRUE)
add_cmake_c_cxx_include_directory(${fmt_SOURCE_DIR}/include TRUE)
add_cmake_c_cxx_include_directory(${plog_SOURCE_DIR}/include TRUE)
add_cmake_c_cxx_include_directory(${random_SOURCE_DIR}/include TRUE)
