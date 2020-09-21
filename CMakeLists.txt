cmake_minimum_required (VERSION 3.10)

if (APPLE)
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
endif()

add_compile_options(-Wall -Werror -Wpedantic -Wextra -Wformat=2 -fPIC)

project (mmotd)
add_executable(mmotd main.cpp http_request.cpp external_network.cpp network.cpp lastlog.cpp)

find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(fmt REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

set_target_properties(mmotd PROPERTIES
                      C_STANDARD 11
                      C_STANDARD_REQUIRED on
                      C_EXTENSIONS off
                      CXX_STANDARD 17
                      CXX_STANDARD_REQUIRED on
                      CXX_EXTENSIONS off)

target_include_directories(mmotd SYSTEM PUBLIC ${Boost_INCLUDE_DIRS} PUBLIC ${JSONCPP_INCLUDE_DIRS})

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    string(APPEND CMAKE_CXX_FLAGS " -stdlib=libc++")
endif()

message("build type: ${CMAKE_BUILD_TYPE}")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DDEBUG -D_DEBUG)
    string(APPEND CMAKE_CXX_FLAGS " -Og -g3 -ggdb3")
    string(APPEND CMAKE_C_FLAGS " -Og -g3 -ggdb3")
else ()
    string(APPEND CMAKE_CXX_FLAGS " -O2")
    string(APPEND CMAKE_C_FLAGS " -O2")
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_link_libraries(mmotd ${Boost_LIBRARIES} fmt::fmt ${JSONCPP_LINK_LIBRARIES})
