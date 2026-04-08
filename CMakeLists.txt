cmake_minimum_required(VERSION 3.24) # Increased for better Apple Silicon/Bundle support
project(MyArxExtension LANGUAGES CXX)

# 1. Project Settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OUTPUT_NAME "MyExtension")

# 2. SDK Path Setup
# Using a variable makes it easier to swap versions later
set(ARX_SDK_PATH "/Users/joshuahal/Developer/Autodesk/ObjectARX 2027")

# 3. Create the Target (macOS Bundle)
# Note: 'MODULUS' is your internal target name; 'MyExtension' is the filename.
add_library(MODULUS MODULE src/main.cpp)

# 4. Include Directories
# Use target_include_directories to keep the scope clean
target_include_directories(MODULUS PRIVATE 
    "${ARX_SDK_PATH}/inc"
)

# 5. Target Properties for macOS .bundle
set_target_properties(MODULUS PROPERTIES
    BUNDLE TRUE
    BUNDLE_EXTENSION "bundle"
    OUTPUT_NAME "${OUTPUT_NAME}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    PREFIX "" # Prevents 'libMyExtension.bundle'
)

# 6. Linking (The 2027 Way)
# In 2027, ensure you are pointing to the /lib-arm64 or /lib-x86_64 folder 
# depending on your target architecture.
target_link_directories(MODULUS PRIVATE "${ARX_SDK_PATH}/lib-arm64")

target_link_libraries(MODULUS PRIVATE
    "-framework Cocoa"
    # For 2027, use the full library names found in the /lib folders
    acdb
    acge
    accore
    # rxapi (Often required for core ARX functionality)
)