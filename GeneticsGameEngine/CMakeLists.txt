# Minimum required version
cmake_minimum_required(VERSION 3.25)

# Project configuration
project(GeneticsGameEngine VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(WindowsSDK REQUIRED)

# Create executable
add_executable(GeneticsGameEngine
    src/core/main.cpp
    src/graphics/GraphicsEngine.cpp
    src/core/Window.cpp
    # ... other source files
)

# Link DirectX 12 libraries
target_link_libraries(GeneticsGameEngine PRIVATE
    d3d12.lib
    dxgi.lib
    d3dcompiler.lib
)

# Compiler optimizations
set_target_properties(GeneticsGameEngine PROPERTIES
    COMPILE_OPTIONS "$<$<CONFIG:Release>:/O2 /GL /Gw /Gy /arch:AVX2>"
    LINK_FLAGS "$<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF>"
)

# Include directories
target_include_directories(GeneticsGameEngine PRIVATE
    src/core
    src/graphics
    src/genetics
)

# Add compile definitions for Windows
target_compile_definitions(GeneticsGameEngine PRIVATE
    WIN32
    _WINDOWS
    UNICODE
    _UNICODE
)

# Set Windows subsystem
set_target_properties(GeneticsGameEngine PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS OFF
    VS_WINRT_COMPONENT OFF
)

# Set output directory
set_target_properties(GeneticsGameEngine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Add precompiled headers if needed
# target_precompile_headers(GeneticsGameEngine PRIVATE
#     src/core/stdafx.h
# )
