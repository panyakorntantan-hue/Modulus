cmake_minimum_required(VERSION 3.24)
project(MyArxExtension LANGUAGES CXX)

# 1. Project & Standard Settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OUTPUT_NAME "MyExtension")

# 2. Path to ObjectARX 2027 SDK
set(ARX_SDK_PATH "/Users/joshuahal/Developer/Autodesk/ObjectARX 2027")

# 3. Create the Target
add_library(MODULUS MODULE src/main.cpp)

# 4. Include Directories
target_include_directories(MODULUS PRIVATE 
    "${ARX_SDK_PATH}/inc"
    "${ARX_SDK_PATH}/inc/PAL/api"  # Platform Abstraction Layer
)

# 5. Compiler Flags for ARX
# ObjectARX on macOS often requires specific flags
target_compile_options(MODULUS PRIVATE
    -fvisibility=hidden
    -fvisibility-inlines-hidden
)

# 6. Link Options - Try These Approaches
# IMPORTANT: ObjectARX on macOS may require linking against the host application
# or using weak linking for certain symbols

target_link_options(MODULUS PRIVATE
    # Try this first - search in SDK root
    -L"${ARX_SDK_PATH}/lib"
    -L"${ARX_SDK_PATH}"
    
    # macOS-specific: Allow undefined symbols (ObjectARX pattern)
    -undefined dynamic_lookup
)

# 7. Link Libraries
# Start minimal - ARX provides symbols at runtime
target_link_libraries(MODULUS PRIVATE
    "-framework Cocoa"
    "-framework CoreFoundation"
)

# If the SDK has pre-built libs, add them:
# Uncomment and adjust if you find .a files:
# target_link_libraries(MODULUS PRIVATE accore acdb)

# 8. Bundle Configuration
set_target_properties(MODULUS PROPERTIES
    BUNDLE TRUE
    BUNDLE_EXTENSION "bundle"
    OUTPUT_NAME "${OUTPUT_NAME}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    PREFIX ""
    # macOS-specific bundle settings
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
)

# 9. Optional: Copy resources if needed
# add_custom_command(TARGET MODULUS POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     "${ARX_SDK_PATH}/res"
#     "$<TARGET_BUNDLE_CONTENT_DIR:MODULUS>/Resources"
# )