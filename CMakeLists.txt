# Blosc - Blocked Shuffling and Compression Library
#
# Copyright (C) 2023  The Blosc Developers <blosc@blosc.org>
# https://blosc.org
# License: GNU Affero General Public License v3.0 (see LICENSE.txt)

cmake_minimum_required(VERSION 3.20)
project(blosc2_grok LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

# Needed when working with cibuildwheel and MacOS
# set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)

# Debug build
# set(CMAKE_C_FLAGS_DEBUG_INIT "-O0 -g")
# set(CMAKE_CXX_FLAGS_DEBUG_INIT "-O0 -g")

message("Building Blosc2 plugin...")

# Propagate CMAKE_OSX_ARCHITECTURES env variable into CMAKE_SYSTEM_PROCESSOR
if(DEFINED ENV{CMAKE_OSX_ARCHITECTURES})
    if("$ENV{CMAKE_OSX_ARCHITECTURES}" STREQUAL "arm64")
        set(CMAKE_SYSTEM_PROCESSOR arm64)
    endif()
endif()

# Find blosc2.h
find_package(Python COMPONENTS Interpreter NumPy Development.Module REQUIRED)
message(STATUS "Executable: ${Python_EXECUTABLE}")
message(STATUS "NumPy found: ${Python_NumPy_FOUND}")
message(STATUS "NumPy version: ${Python_NumPy_VERSION}")
message(STATUS "Python_NumPy_INCLUDE_DIRS: ${Python_NumPy_INCLUDE_DIRS}")
if (UNIX)
    cmake_path(SET Python_ROOT NORMALIZE "${Python_NumPy_INCLUDE_DIRS}/../../../../../..")
else()
    cmake_path(SET Python_ROOT NORMALIZE "${Python_NumPy_INCLUDE_DIRS}/../../../../..")
endif()
cmake_path(SET Python_INCLUDE NORMALIZE "${Python_ROOT}/include")
message(STATUS "Found Python include: ${Python_INCLUDE}")
cmake_path(SET Python_LIB NORMALIZE "${Python_ROOT}/lib")
cmake_path(SET Python_LIB64 NORMALIZE "${Python_ROOT}/lib64")
cmake_path(SET Python_Blosc2_INCLUDE NORMALIZE "${Python_ROOT}/include/blosc2.h")
cmake_path(HAS_FILENAME Python_Blosc2_INCLUDE HAS_BLOSC2)
if(HAS_BLOSC2)
    set(BLOSC2_INCLUDE_DIR ${Python_INCLUDE})
    message(STATUS "Found Blosc2 include: ${Python_Blosc2_INCLUDE}")
else()
    message(FATAL_ERROR "No Blosc2 includes found.  Aborting.")
endif()

include_directories("${BLOSC2_INCLUDE_DIR}")
add_subdirectory(src)
