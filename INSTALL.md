# Table of contents

* [Platforms](#platforms)
* [Get the source code](#get-the-source-code)
* [Dependencies](#dependencies)
  * [Windows dependencies](#windows-dependencies)
  * [macOS dependencies](#macos-dependencies)
  * [Linux dependencies](#linux-dependencies)
* [Compiling](#compiling)
  * [Windows details](#windows-details)
    * [MinGW](#mingw)
  * [macOS details](#macos-details)
  * [Linux details](#linux-details)

# Platforms

You should be able to compile **mmotd** successfully on the following platforms:

* macOS 10.15.5 Catalina (older operating system may work)
* **[UNFINISHED]** Windows 10 + Visual Studio 2019 or [MinGW](#mingw)
* **[UNFINISHED]** Linux Ubuntu 20.04

# Get the source code

You can clone the repository which when configured will download it's dependencies:

```bash
git clone https://github.com/jasonivey/mmotd.git
```

You can use [Git for Windows](https://git-for-windows.github.io/) to clone the repository on Windows.

# Dependencies

To compile **mmotd** you will need:

* A compiler which supports C++17 (i.e. `Visual Studio 19`, `llvm-11` or `gcc-10`)
* A relatively new version of [CMake](https://cmake.org) (3.8 or greater)
* [Ninja](https://ninja-build.org) build system
* The header-only version of the [Boost](https://www.boost.org/) libraries
* The [OpenSSL](https://www.openssl.org/) development library (version 1.1 or greater)
* [OpenSSL](https://www.openssl.org/) depends upon the [Zlib](https://zlib.net/) library

## Windows dependencies

* **[UNFINISHED]**

## macOS dependencies

* [Homebrew](https://brew.sh/) or other package manager
* A compiler which supports C++17 (i.e. `llvm-11` or `gcc-10`)
  * `brew install llvm-11`
  * `brew install gcc-10`
* `brew install cmake ninja boost openssl zlib`

## Linux dependencies

You will need the following dependencies on **Ubuntu/Debian**:

```bash
sudo apt-get install -y g++ cmake ninja-build boost openssl openssl-dev zlib
```

You will need the following dependencies on **Fedora**:

```bash
sudo dnfinstall -y gcc-c++ cmake ninja-build boost openssl openssl-dev zlib
```

# Compiling

1. [Get the code](#get-the-source-code), put it in a folder like:
   1. ***Windows***: `C:\dev\mmotd`
   1. ***macOS*** or ***Linux***: `~/dev/mmotd`
1. Create a `build` directory inside the project which will store all the files that are generated as part of the build (`.exe`, `.lib`, `.obj`, `.a`, `.o`, etc).
   1. ***Windows***: 
      ```cmd
      C:
      cd \dev\mmotd
      md build
      ```
   1. ***macOS*** or ***Linux***:
      ```bash
      cd ~\dev\mmotd
      mkdir build
      ```
1. Enter in the new directory and execute `cmake`
   ```bash
   cd build
   cmake -G Ninja ..
   ```
   Here `cmake` needs different options depending on your platform. You must check the details for [Windows](#windows-details), [macOS](#macos-details), and [Linux](#linux-details).   
   Some `cmake` options can be modified using tools like [`ccmake`](https://cmake.org/cmake/help/latest/manual/ccmake.1.html) or [`cmake-gui`](https://cmake.org/cmake/help/latest/manual/cmake-gui.1.html).
1. After you have executed and configured `cmake`, you have to compile the project
   ```bash
   cd build
   ninja
   ```
1. When `ninja` finishes the compilation, you can find the executable inside the `build/app` directory
   1. ***Windows***: `dir C:\dev\mmotd\build\app\mmotd.exe`
   1. ***macOS*** or ***Linux***: `ls ~/dev/mmotd/build/app/mmotd`

## Windows details

Open a [developer command prompt](https://docs.microsoft.com/en-us/dotnet/framework/tools/developer-command-prompt-for-vs) or in the command line `cmd.exe` call
```cmd
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
```

And then

```cmd
C:
cd \dev\mmotd
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -GNinja ..
ninja
```

### MinGW

Support for the MinGW compiler is non-existant at the moment.  Using it may lead to problems in the compilation process.  If you see that the detected C++ compiler by `cmake` is `C:\MinGW\bin\c++.exe` or something similar it means that the `MinGW` bin directory has higher precedence than the Visual Studio bin directories in the `%PATH%` environment variable.

There are several options to remedy the situation.  It's possible to modify the `%PATH%` environment variable or an ignore path, `CMAKE_IGNORE_PATH`, can be added to the `cmake` command line

```cmd
cmake -DCMAKE_IGNORE_PATH=C:\MinGW\bin ...
```

## macOS details

Run `cmake` with the following parameters and then `ninja`:

```bash
cd ~/dev/mmotd
mkdir build
cd build
CC=/usr/local/opt/llvm/bin/clang CXX=/usr/local/opt/llvm/bin/clang++ cmake -GNinja -DCMAKE_BUILD_TYPE=Debug ..
or
CC=/usr/local/opt/gcc/bin/gcc-10 CXX=/usr/local/opt/gcc/bin/g++-10 cmake -GNinja -DCMAKE_BUILD_TYPE=Debug ..
ninja
```

## Linux details

Run `cmake` with the following parameters and then `ninja`:

```bash
cd ~/dev/mmotd
mkdir build
cd build
CC=clang CXX=clang++ cmake -GNinja -DCMAKE_BUILD_TYPE=Debug ..
or
CC=gcc CXX=g++ cmake -GNinja -DCMAKE_BUILD_TYPE=Debug ..
ninja
```

# Using shared third party libraries
***Note***: for future notes.  Currently everything is static linked against the binary.
