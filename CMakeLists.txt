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

include(GNUInstallDirs)
include(FetchContent)

message("Building Blosc2 plugin...")

# Necessary when calling via cmake (to know which python to call)
find_package(Python REQUIRED COMPONENTS Interpreter) 

# Propagate CMAKE_OSX_ARCHITECTURES env variable into CMAKE_SYSTEM_PROCESSOR
if(DEFINED ENV{CMAKE_OSX_ARCHITECTURES})
    if("$ENV{CMAKE_OSX_ARCHITECTURES}" STREQUAL "arm64")
        set(CMAKE_SYSTEM_PROCESSOR arm64)
    endif()
endif()

# ============================================================
# Step 1: Find C-Blosc2
# ============================================================
# Find blosc2 package location using Python
execute_process(
    COMMAND "${Python_EXECUTABLE}" -c "import blosc2, pathlib; print(pathlib.Path(blosc2.__file__).parent)"
    OUTPUT_VARIABLE BLOSC2_PACKAGE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE BLOSC2_IMPORT_RESULT
)

# To compile blosc2_grok, we only need the include files
if(BLOSC2_IMPORT_RESULT EQUAL 0 AND EXISTS "${BLOSC2_PACKAGE_DIR}")
    message(STATUS "Found blosc2 package at: ${BLOSC2_PACKAGE_DIR}")

    # Try only PEP 427 compliant structure (blosc2/include/)
    set(BLOSC2_INCLUDE_DIR "${BLOSC2_PACKAGE_DIR}/include")
    if(NOT EXISTS "${BLOSC2_INCLUDE_DIR}/blosc2.h")
        message(FATAL_ERROR "blosc2 package found but blosc2.h not found. Tried:\n  ${BLOSC2_PACKAGE_DIR}/include")
    else()
        message(STATUS "Found Blosc2 include: ${BLOSC2_INCLUDE_DIR}/blosc2.h")
    endif()
else()
    message(FATAL_ERROR "Could not import blosc2 package. Please install it first: pip install blosc2")
endif()

# For the test_grok and roundtrip executables, we also need the actual blosc2 lib
# Try only PEP 427 compliant structure (blosc2/lib/)
set(BLOSC2_LIB_DIR "${BLOSC2_PACKAGE_DIR}/lib")
if(NOT EXISTS "${BLOSC2_LIB_DIR}")
    message(FATAL_ERROR "blosc2 lib dir not found. Tried:\n  ${BLOSC2_PACKAGE_DIR}/lib")
endif()

find_library(BLOSC2_LIBRARY
    NAMES blosc2 libblosc2
    PATHS "${BLOSC2_LIB_DIR}"
    NO_DEFAULT_PATH
)

if(BLOSC2_LIBRARY)
    message(STATUS "Found Blosc2 library: ${BLOSC2_LIBRARY}")
    set(BLOSC2_LIBRARIES "${BLOSC2_LIBRARY}")
    get_filename_component(BLOSC2_LIBRARY_DIR_RESOLVED "${BLOSC2_LIBRARY}" DIRECTORY)
else()
    message(FATAL_ERROR "Blosc2 library not found in ${BLOSC2_LIB_DIR}. Please install the blosc2 Python package with bundled libraries.")
endif()

# ============================================================
# blosc2_grok plugin source files
# ============================================================
add_subdirectory(src)