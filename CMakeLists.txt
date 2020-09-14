cmake_minimum_required (VERSION 2.6)
project (mmotd-binary)
add_executable(mmotd main.cpp network.cpp lastlog.cpp)

find_package(Boost REQUIRED COMPONENTS system filesystem)
include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -O0 -g -ggdb3 -DDEBUG")

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_link_libraries(mmotd ${Boost_LIBRARIES})
