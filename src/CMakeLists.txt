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
# We will be using SHARED or MODULE depending on the platform.
# Ideally, we would like to use SHARED for all platforms
# because that allows to link with C++ code in the shared library.
# Unfortunately, not every platform supports SHARED.
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

include_directories(
    grok/src/lib/core
    ${CMAKE_CURRENT_BINARY_DIR}/grok/src/lib/core
)

target_link_libraries(blosc2_grok grokj2k ${BLOSC2_LIBRARIES})
if(DEFINED BLOSC2_LIBRARY_DIR_RESOLVED)
    # Ensure repair tools can locate libblosc2 via embedded rpath.
    set_target_properties(blosc2_grok PROPERTIES
        BUILD_RPATH "${BLOSC2_LIBRARY_DIR_RESOLVED}"
    )
    # Set platform-specific INSTALL_RPATH
    if(APPLE)
        set_target_properties(blosc2_grok PROPERTIES
            INSTALL_RPATH "@loader_path/.dylibs"
        )
    elseif(UNIX)
        set_target_properties(blosc2_grok PROPERTIES
            INSTALL_RPATH "$ORIGIN/.libs"
        )
    endif()
endif()

link_directories(${Python_LIB})
if (UNIX AND NOT APPLE)
    link_directories(${Python_LIB64})
endif (UNIX AND NOT APPLE)


# Install
install(TARGETS blosc2_grok
    LIBRARY DESTINATION blosc2_grok
    RUNTIME DESTINATION blosc2_grok
    ARCHIVE DESTINATION blosc2_grok
)
install(FILES blosc2_grok.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT DEV)

# Bundle libblosc2 library for wheel repair
if(APPLE AND BLOSC2_LIBRARY)
    # macOS: Install dylibs to .dylibs directory
    get_filename_component(BLOSC2_LIB_DIR "${BLOSC2_LIBRARY}" DIRECTORY)
    file(GLOB BLOSC2_DYLIBS "${BLOSC2_LIB_DIR}/libblosc2*.dylib")
    foreach(DYLIB ${BLOSC2_DYLIBS})
        get_filename_component(DYLIB_REALPATH "${DYLIB}" REALPATH)
        install(FILES "${DYLIB_REALPATH}" DESTINATION blosc2_grok/.dylibs)
    endforeach()
elseif(UNIX AND NOT APPLE AND BLOSC2_LIBRARY)
    # Linux: Install shared libraries to .libs directory for auditwheel
    get_filename_component(BLOSC2_LIB_DIR "${BLOSC2_LIBRARY}" DIRECTORY)
    file(GLOB BLOSC2_SOS "${BLOSC2_LIB_DIR}/libblosc2.so*")
    foreach(SO ${BLOSC2_SOS})
        if(EXISTS "${SO}")
            get_filename_component(SO_REALPATH "${SO}" REALPATH)
            install(FILES "${SO_REALPATH}" DESTINATION blosc2_grok/.libs)
            message(STATUS "Will install blosc2 SO: ${SO_REALPATH}")
        endif()
    endforeach()
elseif(WIN32 AND BLOSC2_LIBRARY)
    # Windows: Install DLL to same directory as plugin
    get_filename_component(BLOSC2_LIB_DIR "${BLOSC2_LIBRARY}" DIRECTORY)
    message(STATUS "Windows: BLOSC2_LIBRARY = ${BLOSC2_LIBRARY}")
    message(STATUS "Windows: BLOSC2_LIB_DIR = ${BLOSC2_LIB_DIR}")
    
    # Search common locations for blosc2.dll
    # Note: BLOSC2_LIBRARY points to the .lib file, we need to find the .dll
    set(_blosc2_dll_patterns
        # Common conda/system install locations
        "${BLOSC2_LIB_DIR}/../bin/blosc2.dll"
        "${BLOSC2_LIB_DIR}/../Library/bin/blosc2.dll"
        # Pip wheel install locations (site-packages structure)
        "${BLOSC2_LIB_DIR}/blosc2.dll"
        "${BLOSC2_LIB_DIR}/../Scripts/blosc2.dll"
        "${BLOSC2_LIB_DIR}/../../Scripts/blosc2.dll"
        # Try to find via Python import (for pip-installed blosc2)
        "${BLOSC2_LIB_DIR}/../../../blosc2/blosc2.dll"
        "${BLOSC2_LIB_DIR}/../../blosc2.dll"
    )
    
    message(STATUS "Windows: Searching for blosc2.dll in patterns:")
    foreach(PATTERN ${_blosc2_dll_patterns})
        message(STATUS "  ${PATTERN}")
    endforeach()
    
    set(_found_blosc2_dll FALSE)
    foreach(DLL_PATTERN ${_blosc2_dll_patterns})
        file(GLOB BLOSC2_DLLS "${DLL_PATTERN}")
        foreach(DLL ${BLOSC2_DLLS})
            if(EXISTS "${DLL}")
                get_filename_component(DLL_REALPATH "${DLL}" REALPATH)
                install(FILES "${DLL_REALPATH}" DESTINATION blosc2_grok)
                message(STATUS "Will install blosc2 DLL: ${DLL_REALPATH}")
                set(_found_blosc2_dll TRUE)
                break()
            endif()
        endforeach()
        if(_found_blosc2_dll)
            break()
        endif()
    endforeach()
    
    # If still not found, try to find it via Python
    if(NOT _found_blosc2_dll)
        message(STATUS "Windows: Trying to find blosc2.dll via Python import...")
        execute_process(
            COMMAND "${Python_EXECUTABLE}" -c "import blosc2, pathlib; print(pathlib.Path(blosc2.__file__).parent / 'blosc2.dll')"
            OUTPUT_VARIABLE _blosc2_dll_from_python
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        message(STATUS "Windows: Python returned: ${_blosc2_dll_from_python}")
        if(_blosc2_dll_from_python AND EXISTS "${_blosc2_dll_from_python}")
            get_filename_component(DLL_REALPATH "${_blosc2_dll_from_python}" REALPATH)
            install(FILES "${DLL_REALPATH}" DESTINATION blosc2_grok)
            message(STATUS "Will install blosc2 DLL (via Python): ${DLL_REALPATH}")
            set(_found_blosc2_dll TRUE)
        endif()
    endif()
    
    if(NOT _found_blosc2_dll)
        message(WARNING "blosc2.dll not found - wheel may not work on Windows without blosc2 installed")
    endif()
endif()

# Test program
if(NOT DEFINED ENV{DONT_BUILD_EXAMPLES})
    message(STATUS "DONT_BUILD_EXAMPLES not set-> Building examples")
    add_executable(test_grok test_grok.cpp blosc2_grok.cpp utils.cpp)
    add_executable(roundtrip roundtrip.cpp blosc2_grok.cpp utils.cpp)
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
