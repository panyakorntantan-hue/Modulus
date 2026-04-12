cmake_minimum_required(VERSION 3.16)
project(ModulusLite LANGUAGES CXX)

# ============================================================================
# GLOBAL SETTINGS
# ============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

# Enable Qt auto tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ============================================================================
# DEPENDENCIES
# ============================================================================

# ObjectARX SDK (UPDATE THIS TO ABSOLUTE PATH IF NEEDED)
set(OBJECTARX_SDK_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/../ObjectARX2027Mac"
    CACHE PATH "Path to ObjectARX 2027 Mac SDK"
)

# Debug paths (useful during setup)
message(STATUS "OBJECTARX_SDK_PATH = ${OBJECTARX_SDK_PATH}")
message(STATUS "ARX INC PATH       = ${OBJECTARX_SDK_PATH}/inc")

# Qt5 (Homebrew path)
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt@5")
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

# ============================================================================
# CORE LIBRARY (HEADER-ONLY)
# ============================================================================

add_library(ModulusLiteCore INTERFACE)

target_sources(ModulusLiteCore INTERFACE
    core.h
)

target_include_directories(ModulusLiteCore INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ============================================================================
# CAD ADAPTER (ObjectARX)
# ============================================================================

add_library(ModulusLiteCAD STATIC
    cad_adapter.cpp
    cad_adapter.h
)

target_include_directories(ModulusLiteCAD PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OBJECTARX_SDK_PATH}/inc
    ${OBJECTARX_SDK_PATH}/inc/PAL
    ${OBJECTARX_SDK_PATH}/inc/PAL/api
)

target_link_directories(ModulusLiteCAD PUBLIC
    ${OBJECTARX_SDK_PATH}/lib
)

target_link_libraries(ModulusLiteCAD PUBLIC
    ModulusLiteCore
    "-framework CoreFoundation"
    "-framework Cocoa"
)

# ============================================================================
# UI LIBRARY (Qt)
# ============================================================================

add_library(ModulusLiteUI STATIC
    ui_panel.cpp
    ui_panel.h
)

target_include_directories(ModulusLiteUI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ModulusLiteUI PUBLIC
    ModulusLiteCore
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

# ============================================================================
# PLUGIN (AutoCAD Bundle)
# ============================================================================

add_library(ModulusLitePlugin MODULE
    plugin_main.cpp
)

target_include_directories(ModulusLitePlugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OBJECTARX_SDK_PATH}/inc
    ${OBJECTARX_SDK_PATH}/inc/PAL
    ${OBJECTARX_SDK_PATH}/inc/PAL/api
)

target_link_directories(ModulusLitePlugin PRIVATE
    ${OBJECTARX_SDK_PATH}/lib
)

target_link_libraries(ModulusLitePlugin PRIVATE
    ModulusLiteCore
    ModulusLiteCAD
    ModulusLiteUI
    "-framework CoreFoundation"
    "-framework Cocoa"
)

set_target_properties(ModulusLitePlugin PROPERTIES
    PREFIX ""
    SUFFIX ".bundle"
    BUNDLE FALSE
    MACOSX_BUNDLE FALSE
)

# Copy plugin after build
add_custom_command(TARGET ModulusLitePlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:ModulusLitePlugin>
        ${CMAKE_BINARY_DIR}/bin/ModulusLite.bundle
)

# ============================================================================
# TESTS
# ============================================================================

enable_testing()

add_executable(ModulusLiteTests
    tests/test_core.cpp
    tests/test_routing.cpp
)

target_include_directories(ModulusLiteTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ModulusLiteTests PRIVATE
    ModulusLiteCore
)

add_test(NAME ModulusLiteTests COMMAND ModulusLiteTests)

# ============================================================================
# INSTALL
# ============================================================================

install(TARGETS ModulusLitePlugin
    LIBRARY DESTINATION
    "Library/Application Support/Autodesk/ApplicationPlugins"
)

install(FILES
    README.md
    LICENSE
    DESTINATION "."
)