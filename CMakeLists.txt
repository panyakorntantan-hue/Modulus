cmake_minimum_required(VERSION 3.16)
project(ModulusLite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt auto tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON) # Added UIC in case you use .ui files

if(MSVC)
    add_definitions(
        /W4 /MP
        -D_CRT_SECURE_NO_WARNINGS  # Silences mbstowcs warnings
        -D_WINSOCK2API_
        -D_UNICODE -DUNICODE
        -D_AFXDLL
        -D_AC25_
        -D_WIN64
    )
    # Disable 4201 (nameless structs) for ObjectARX compatibility
    add_compile_options(/wd4201) 
    
    # Keep /WX if you want high code quality, but maybe disable it 
    # until you've cleared the initial conversion warnings.
    # add_compile_options(/WX) 
endif()

# ============================================================================
# DEPENDENCIES
# ============================================================================

set(OBJECTARX_SDK_PATH "C:/CDROM1" CACHE PATH "Path to ObjectARX SDK")

# Qt5 - Ensure this path points to the MSVC 64-bit version
list(APPEND CMAKE_PREFIX_PATH "C:/Users/admin/Downloads/Modulus-main/5.15.2/msvc2019_64")
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

# ============================================================================
# TARGETS
# ============================================================================

# Core (Interface)
add_library(ModulusLiteCore INTERFACE)
target_sources(ModulusLiteCore INTERFACE core.h)
target_include_directories(ModulusLiteCore INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# CAD Adapter
add_library(ModulusLiteCAD STATIC cad_adapter.cpp cad_adapter.h)
target_include_directories(ModulusLiteCAD PUBLIC 
    ${OBJECTARX_SDK_PATH}/inc-x64 
    ${OBJECTARX_SDK_PATH}/inc
)
target_link_libraries(ModulusLiteCAD PUBLIC ModulusLiteCore)

# UI Library
add_library(ModulusLiteUI STATIC panel.cpp panel.h)
target_link_libraries(ModulusLiteUI PUBLIC ModulusLiteCore Qt5::Widgets)

# ============================================================================
# PLUGIN (The actual .arx)
# ============================================================================

# Changed target name to match your install/property logic
add_library(ModulusLitePlugin MODULE
    plugin_main.cpp
    exports.def # Ensure this file contains 'acrxGetApiVersion' and 'acrxEntryPoint'
)

target_link_directories(ModulusLitePlugin PRIVATE 
    ${OBJECTARX_SDK_PATH}/lib-x64
)

target_link_libraries(ModulusLitePlugin PRIVATE
    ModulusLiteCore
    ModulusLiteCAD
    ModulusLiteUI
    # Standard ARX libs (Check your SDK version, some need '25' suffix)
    accore.lib
    acad.lib
    acdb25.lib 
    acge25.lib
    acui25.lib
    adui25.lib
)

set_target_properties(ModulusLitePlugin PROPERTIES
    PREFIX ""
    SUFFIX ".arx"
    OUTPUT_NAME "ModulusLite"
    LINK_FLAGS "${ARX_LINKER_FLAGS}"
)

# ============================================================================
# POST-BUILD & INSTALL
# ============================================================================

install(TARGETS ModulusLitePlugin
    LIBRARY DESTINATION bin
    RUNTIME DESTINATION bin
)

# Copy to a convenient 'out' folder
add_custom_command(TARGET ModulusLitePlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ModulusLitePlugin> ${CMAKE_BINARY_DIR}/bin/ModulusLite.arx
)