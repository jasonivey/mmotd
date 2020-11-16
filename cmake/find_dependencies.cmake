# cmake/find_dependencies.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.10)

if (APPLE)
    # enables setting ${package}_root variables like ZLIB_ROOT without warning
    if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12")
        cmake_policy(SET CMP0074 NEW)
    endif ()
    set (ZLIB_ROOT /usr/local/opt/zlib)
endif ()
find_package(ZLIB 1.2.11 REQUIRED)

find_package(fmt)
find_package(Boost 1.74.0 REQUIRED)

# Find OpenSSL for use with boost::asio and boost::beast.
#  There is also a submodule certify which handles the https
#  certificates.
find_package(OpenSSL 1.1.1 REQUIRED)

# Update the submodules which are strictly stored in mmotd/external.
#  The reason there are as many as there are is due to the fact that
#  when using C++17 it breaks ABI compatibility with libraries which
#  are only build with C++11.
find_package(Git)
if (GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if (GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if (NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with '${GIT_SUBMOD_RESULT}', please checkout submodules")
        endif ()
    endif ()
endif ()

