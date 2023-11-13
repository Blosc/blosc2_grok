# Blosc - Blocked Shuffling and Compression Library
#
# Copyright (C) 2023  The Blosc Developers <blosc@blosc.org>
# https://blosc.org
# License: BSD 3-Clause (see LICENSE.txt)
#
# See LICENSE.txt for details about copyright and rights to use.

set(GRK_BUILD_LIBPNG OFF)
set(GRK_BUILD_LIBTIFF OFF)
set(GRK_BUILD_LCMS2 ON)
set(GRK_BUILD_JPEG OFF)

# Build statically
#set(BUILD_SHARED_LIBS OFF)
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(APPLE)
    # Apple Rosetta cannot emulate AVX due to Intel patents
    set(ENABLE_AVX2 OFF)
endif()
add_subdirectory(grok)

# Build libblosc2_grok.so
add_library(blosc2_grok MODULE blosc2_grok.cpp)

if(MSVC OR MINGW)
    # Necessary for compiling blosc2_grok.lib
    set_property(TARGET blosc2_grok PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

include_directories(
    grok/src/lib/core
    ${CMAKE_CURRENT_BINARY_DIR}/grok/src/lib/core
)

target_link_libraries(blosc2_grok grokj2k)

link_directories(${Python_LIB})
if (UNIX AND NOT APPLE)
    link_directories(${Python_LIB64})
endif (UNIX AND NOT APPLE)


# Install
install(TARGETS blosc2_grok LIBRARY DESTINATION blosc2_grok)

# Test program
add_executable(test_grok test_grok.cpp blosc2_grok.cpp)
if(MSVC OR MINGW)
   target_link_libraries(test_grok blosc2 grokj2k)
else()
   target_link_libraries(test_grok blosc2 grokj2k m)
endif()
