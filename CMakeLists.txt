cmake_minimum_required (VERSION 3.10)

if (APPLE)
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
endif()

add_compile_options(-Wall -Werror -Wpedantic -Wextra -Wformat=2 -fPIC)

project (mmotd)
add_executable(mmotd external_network.cpp http_request.cpp http_secure_request.cpp lastlog.cpp logging.cpp main.cpp network.cpp options.cpp system.cpp)

#find_package(Boost COMPONENTS program_options log_setup log atomic chrono date_time filesystem regex thread REQUIRED)
find_package(Boost COMPONENTS program_options log_setup log REQUIRED)
find_package(fmt REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

#find_package(OpenSSL REQUIRED)
#if (OPENSSL_FOUND)
  # Add the include directories for compiling
#  target_include_directories(${PROJECT_NAME} PUBLIC )
  # Add the static lib for linking
#  target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)
message(STATUS "Found OpenSSL ${OPENSSL_CRYPTO_LIBRARY}")
message(STATUS "Found OpenSSL ${OPENSSL_CRYPTO_LIBRARIES}")
message(STATUS "Found OpenSSL ${OPENSSL_SSL_LIBRARY}")
message(STATUS "Found OpenSSL ${OPENSSL_SSL_LIBRARIES}")
message(STATUS "Found OpenSSL ${OPENSSL_LIBRARIES}")
message(STATUS "Found OpenSSL ${OPENSSL_VERSION}")
message(STATUS "Found OpenSSL ${OPENSSL_APPLINK_SOURCE}")
#else()
#  message(STATUS "OpenSSL Not Found")
#endif()

set_target_properties(mmotd PROPERTIES
                      C_STANDARD 11
                      C_STANDARD_REQUIRED on
                      C_EXTENSIONS off
                      CXX_STANDARD 17
                      CXX_STANDARD_REQUIRED on
                      CXX_EXTENSIONS off)

target_include_directories(mmotd SYSTEM PUBLIC ${Boost_INCLUDE_DIRS} PUBLIC ${OPENSSL_INCLUDE_DIR} PUBLIC ${JSONCPP_INCLUDE_DIRS})

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    string(APPEND CMAKE_CXX_FLAGS " -stdlib=libc++")
endif()

message("build type: ${CMAKE_BUILD_TYPE}")

add_definitions(-DBOOST_LOG_DYN_LINK )

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DDEBUG -D_DEBUG)
    string(APPEND CMAKE_CXX_FLAGS " -Og -g3 -ggdb3")
    string(APPEND CMAKE_C_FLAGS " -Og -g3 -ggdb3")
else ()
    string(APPEND CMAKE_CXX_FLAGS " -O2")
    string(APPEND CMAKE_C_FLAGS " -O2")
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#set(PROJECT_LIBS ${Boost_LIBRARIES} fmt::fmt ${JSONCPP_LINK_LIBRARIES})
#target_link_libraries(${PROJECT_NAME} -Wl,--start-group  ${PROJECT_LIBS} -Wl,--end-group)
target_link_libraries(mmotd "-framework CoreFoundation" "-framework Security" ${Boost_LIBRARIES} fmt::fmt ${JSONCPP_LINK_LIBRARIES} ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY} ZLIB::ZLIB)
