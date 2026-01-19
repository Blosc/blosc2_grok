# Blosc - Blocked Shuffling and Compression Library
#
# Copyright (C) 2023  The Blosc Developers <blosc@blosc.org>
# https://blosc.org
# License: GNU Affero General Public License v3.0 (see LICENSE.txt)

cmake_minimum_required(VERSION 3.20)
project(blosc2_grok LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

# Grok needs MacOS 11.0 or later
set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0)

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

set(HAS_BLOSC2 FALSE)
set(_blosc2_include_candidates "")
if(DEFINED BLOSC2_INCLUDE_DIR)
    list(APPEND _blosc2_include_candidates "${BLOSC2_INCLUDE_DIR}")
endif()

if (UNIX)
    cmake_path(SET Python_ROOT NORMALIZE "${Python_NumPy_INCLUDE_DIRS}/../../../../../..")
else()
    cmake_path(SET Python_ROOT NORMALIZE "${Python_NumPy_INCLUDE_DIRS}/../../../../..")
endif()
cmake_path(SET Python_INCLUDE NORMALIZE "${Python_ROOT}/include")
message(STATUS "Found Python include: ${Python_INCLUDE}")
cmake_path(SET Python_LIB NORMALIZE "${Python_ROOT}/lib")
cmake_path(SET Python_LIB64 NORMALIZE "${Python_ROOT}/lib64")

# Prefer site-packages/include derived from NumPy's include location.
foreach(_numpy_inc IN LISTS Python_NumPy_INCLUDE_DIRS)
    if(_numpy_inc)
        cmake_path(SET _numpy_site_packages NORMALIZE "${_numpy_inc}/../../..")
        cmake_path(SET _numpy_site_include NORMALIZE "${_numpy_site_packages}/include")
        list(APPEND _blosc2_include_candidates "${_numpy_site_include}")
    endif()
endforeach()

# Fall back to the Python include dir in case blosc2 headers were installed there.
list(APPEND _blosc2_include_candidates "${Python_INCLUDE}")

# Additional fallback: ask Python for site-packages paths.
execute_process(
    COMMAND "${Python_EXECUTABLE}" -c [=[
import sysconfig
import site
import pathlib

paths = []
for p in (sysconfig.get_path("platlib"), sysconfig.get_path("purelib")):
    if p:
        paths.append(p)
if hasattr(site, "getsitepackages"):
    for p in site.getsitepackages():
        if p:
            paths.append(p)

seen = set()
unique_paths = []
for p in paths:
    if p not in seen:
        seen.add(p)
        unique_paths.append(p)

print(";".join(str(pathlib.Path(p) / "include") for p in unique_paths))
]=]
    OUTPUT_VARIABLE BLOSC2_INCLUDE_CANDIDATES
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
foreach(_candidate ${BLOSC2_INCLUDE_CANDIDATES})
    if(_candidate)
        list(APPEND _blosc2_include_candidates "${_candidate}")
    endif()
endforeach()

list(REMOVE_DUPLICATES _blosc2_include_candidates)

find_path(BLOSC2_INCLUDE_DIR
    NAMES blosc2.h
    PATHS ${_blosc2_include_candidates}
    NO_DEFAULT_PATH
)

if(BLOSC2_INCLUDE_DIR)
    set(HAS_BLOSC2 TRUE)
    message(STATUS "Found Blosc2 include: ${BLOSC2_INCLUDE_DIR}/blosc2.h")
else()
    message(FATAL_ERROR "No Blosc2 includes found.  Aborting.")
endif()

include_directories("${BLOSC2_INCLUDE_DIR}")
add_subdirectory(src)
