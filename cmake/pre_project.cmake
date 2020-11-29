# cmake/pre-project.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.8)

# The following was an attempt to disable the automagic adding of
#  --isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk
#  when using a brew installed version of GCC (i.e. gcc-10).  When
#  using a non-system version of GCC it still sets up the --isysroot.
#  This has the negative affect of attempting to use the 'include' and 'lib'
#  directories within the platform SDK.  ERROR, ERROR, ERROR...
set (CMAKE_OSX_SYSROOT "/" CACHE STRING "")
set (CMAKE_OSX_DEPLOYMENT_TARGET "" CACHE STRING "")

# Common compiler options which need to be set for every C++ module
add_compile_options(-std=c++17 -Wall -Werror -Wpedantic -Wextra -Wformat=2 -fPIC)

