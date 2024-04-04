##############################################################################
# blosc2_grok: Grok (JPEG2000 codec) plugin for Blosc2
#
# Copyright (c) 2023  The Blosc Development Team <blosc@blosc.org>
# https://blosc.org
# License: GNU Affero General Public License v3.0 (see LICENSE.txt)
##############################################################################

set(GRK_BUILD_LIBPNG OFF)
set(GRK_BUILD_JPEG OFF)
set(GRK_BUILD_CORE_EXAMPLES OFF)

# Needed in the continuous integration
#if(DEFINED ENV{CMAKE_OSX_ARCHITECTURES})
#    if("$ENV{CMAKE_OSX_ARCHITECTURES}" STREQUAL "arm64")
#        set(GRK_BUILD_LIBTIFF ON)
#        set(GRK_BUILD_LIBPNG ON)
#        set(GRK_BUILD_JPEG ON)
#        set(GRK_BUILD_LCMS2 ON)
#    endif()
#endif()

# Avoid adding unneeded files to the wheel
set(GRK_BUILD_PKGCONFIG_FILES OFF)
set(GRK_BUILD_CODEC OFF)

# Build statically
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if (APPLE)
    # Apple Rosetta cannot emulate AVX due to Intel patents
    set(ENABLE_AVX2 OFF)
endif()
set(CMAKE_FIND_FRAMEWORK NEVER)
add_subdirectory(grok)

# Build libblosc2_grok.so
if (UNIX AND NOT APPLE)  # Linux
    add_library(blosc2_grok SHARED blosc2_grok.cpp)
else if (APPLE)
    if ({CMAKE_OSX_ARCHITECTURES} STREQUAL "arm64")
        add_library(blosc2_grok SHARED blosc2_grok.cpp)
    else()
        add_library(blosc2_grok MODULE blosc2_grok.cpp)
    endif()
else()  # Windows
    add_library(blosc2_grok MODULE blosc2_grok.cpp)
endif()

if (MSVC OR MINGW)
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
install(FILES blosc2_grok.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT DEV)

# Test program
if(NOT DEFINED ENV{DONT_BUILD_EXAMPLES})
    message(STATUS "DONT_BUILD_EXAMPLES not set-> Building examples")
    add_executable(test_grok test_grok.cpp blosc2_grok.cpp utils.cpp)
    add_executable(roundtrip roundtrip.cpp blosc2_grok.cpp utils.cpp)
    if(MSVC OR MINGW)
        target_link_libraries(test_grok blosc2 grokj2k)
        target_link_libraries(roundtrip blosc2 grokj2k)
    else()
        target_link_libraries(test_grok blosc2 grokj2k m)
        target_link_libraries(roundtrip blosc2 grokj2k m)
    endif()
else()
    message(STATUS "DONT_BUILD_EXAMPLES set")
endif()
