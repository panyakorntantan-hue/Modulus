cmake_minimum_required(VERSION 3.16)
project(ModulusLite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt auto tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ============================================================================
# GLOBAL COMPILER CONFIG (MSVC)
# ============================================================================
if(MSVC)
    add_compile_definitions(
        WIN32_LEAN_AND_MEAN
        _WINSOCK2API_         
        _WINSOCKAPI_          # Block Winsock 1.0 globally
        NOMINMAX
        _WIN32_WINNT=0x0A00   
        _CRT_SECURE_NO_WARNINGS
        _UNICODE 
        UNICODE
        _AFXDLL               
        _AC25_                
        _WIN64
    )

    add_compile_options(
        /W4                   
        /MP                   
        /wd4201               
    )
endif()

# ============================================================================
# DEPENDENCIES
# ============================================================================
set(OBJECTARX_SDK_PATH "C:/CDROM1" CACHE PATH "Path to ObjectARX SDK")

# Qt5
list(APPEND CMAKE_PREFIX_PATH "C:/Users/admin/Downloads/Modulus-main/5.15.2/msvc2019_64")
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

# ============================================================================
# TARGET DEFINITIONS
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
    target_compile_options(ModulusLiteUI PRIVATE 
        "/Dbyte=unsigned char"
        "/DByte=unsigned char"
        "/DWord=unsigned short"
    )

# ============================================================================
# THE NUCLEAR FIX (Must come AFTER add_library)
# ============================================================================
if(MSVC)
    # This prevents the 'Byte' conflict in in6addr.h for the UI library
    target_compile_options(ModulusLiteUI PRIVATE "/Dbyte=unsigned char")
    target_compile_definitions(ModulusLiteUI PRIVATE QT_NO_KEYWORDS)
endif()

# ============================================================================
# PLUGIN (The actual .arx)
# ============================================================================
add_library(ModulusLitePlugin MODULE
    plugin_main.cpp
    exports.def 
)

target_link_directories(ModulusLitePlugin PRIVATE 
    ${OBJECTARX_SDK_PATH}/lib-x64
)

target_link_libraries(ModulusLitePlugin PRIVATE
    ModulusLiteCore
    ModulusLiteCAD
    ModulusLiteUI
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
)

# ============================================================================
# POST-BUILD & INSTALL
# ============================================================================
install(TARGETS ModulusLitePlugin DESTINATION bin)

add_custom_command(TARGET ModulusLitePlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ModulusLitePlugin> ${CMAKE_BINARY_DIR}/bin/ModulusLite.arx
)