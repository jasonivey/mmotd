# cmake/pre-project.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.8)

# The following was an attempt to disable the automagic adding of
#  --isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk
#  when using a brew installed version of GCC (i.e. gcc-10).  When
#  using a non-system version of GCC it still sets up the --isysroot.
#  This has the negative affect of attempting to use the 'include' and 'lib'
#  directories within the platform SDK.  ERROR, ERROR, ERROR...

# It turns out that when upgrading to Bug Sur the following two lines uncommented
#  actually break the builtin clang/clang++ and the brew clang/clang++ and brew gcc/g++.
#  Leaving them undefined allows the CMake subsystem to properly find the system
#  SDK.
#set (CMAKE_OSX_SYSROOT "/" CACHE STRING "")
#set (CMAKE_OSX_DEPLOYMENT_TARGET "" CACHE STRING "")

