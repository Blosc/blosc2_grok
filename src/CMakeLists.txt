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

# Avoid adding unneeded files to the wheel
set(GRK_BUILD_PKGCONFIG_FILES OFF)
set(GRK_BUILD_CODEC OFF)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# Build statically
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if (APPLE)
    # Apple Rosetta cannot emulate AVX due to Intel patents
    set(ENABLE_AVX2 OFF)
endif()
set(CMAKE_FIND_FRAMEWORK NEVER)

add_subdirectory(grok)
 # explicitly include headers as not exported by grok
target_include_directories(grokj2k
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/grok/src/lib/core  # source headers
        ${CMAKE_CURRENT_BINARY_DIR}/grok/src/lib/core  # generated headers
)
# Build libblosc2_grok.so
if (UNIX AND NOT APPLE)  # Linux
    add_library(blosc2_grok SHARED blosc2_grok.cpp)
elseif (APPLE)
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

target_include_directories(blosc2_grok PUBLIC ${BLOSC2_INCLUDE_DIR})
# link to grok (not to blosc2, since blosc2_grok always called via blosc2 as plugin)
target_link_libraries(blosc2_grok grokj2k ${BLOSC2_LIBRARIES})

# Add runtime path just in case (e.g. load plugin before blosc2 for example)
# will not always be used, but if can't find blosc2 gives another thing to check
# before failing
if(UNIX AND NOT APPLE)
    set_target_properties(blosc2_grok PROPERTIES
        INSTALL_RPATH "$ORIGIN/../blosc2/lib"
    )
elseif(APPLE)
    set_target_properties(blosc2_grok PROPERTIES
        INSTALL_RPATH "@loader_path/../blosc2/lib"
    )
endif()

# Install
install(TARGETS blosc2_grok
    RUNTIME DESTINATION blosc2_grok
    LIBRARY DESTINATION blosc2_grok
)
# Note: blosc2_grok.h and grok headers are not installed in the wheel
# Python users don't need them, and C developers should build from source

# Test program
if(NOT DEFINED ENV{DONT_BUILD_EXAMPLES})
    message(STATUS "DONT_BUILD_EXAMPLES not set-> Building examples")
    add_executable(test_grok test_grok.cpp blosc2_grok.cpp utils.cpp)
    target_include_directories(test_grok PRIVATE ${BLOSC2_INCLUDE_DIR})
    add_executable(roundtrip roundtrip.cpp blosc2_grok.cpp utils.cpp)
    target_include_directories(roundtrip PRIVATE ${BLOSC2_INCLUDE_DIR})
    if(MSVC OR MINGW)
        target_link_libraries(test_grok ${BLOSC2_LIBRARIES} grokj2k)
        target_link_libraries(roundtrip ${BLOSC2_LIBRARIES} grokj2k)
    else()
        target_link_libraries(test_grok ${BLOSC2_LIBRARIES} grokj2k m)
        target_link_libraries(roundtrip ${BLOSC2_LIBRARIES} grokj2k m)
    endif()
    if(DEFINED BLOSC2_LIBRARY_DIR_RESOLVED)
        set_target_properties(test_grok PROPERTIES
            BUILD_RPATH "${BLOSC2_LIBRARY_DIR_RESOLVED}"
            INSTALL_RPATH "${BLOSC2_LIBRARY_DIR_RESOLVED}"
        )
        set_target_properties(roundtrip PROPERTIES
            BUILD_RPATH "${BLOSC2_LIBRARY_DIR_RESOLVED}"
            INSTALL_RPATH "${BLOSC2_LIBRARY_DIR_RESOLVED}"
        )
    endif()
else()
    message(STATUS "DONT_BUILD_EXAMPLES set")
endif()
