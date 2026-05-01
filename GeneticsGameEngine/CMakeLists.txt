# Minimum required version
cmake_minimum_required(VERSION 3.25)

# Project configuration
project(GeneticsGameEngine VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check for DirectX 12 headers
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/core/dx12_test.h")
    message(FATAL_ERROR "DirectX 12 headers not found. Please ensure Windows SDK and DirectX SDK are installed.")
endif()

# Add source directory
add_subdirectory(src)

# Compiler optimizations
set_target_properties(GeneticsGameEngine PROPERTIES
    COMPILE_OPTIONS "$<$<CONFIG:Release>:/O2 /GL /Gw /Gy>"
)

# Set output directory
set_target_properties(GeneticsGameEngine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Add precompiled headers if needed
# target_precompile_headers(GeneticsGameEngine PRIVATE
#     src/core/stdafx.h
# )
