# Main source directory CMake configuration

# Add subdirectories
add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(genetics)
add_subdirectory(engine)

# Create executable
add_executable(GeneticsGameEngine
    core/main.cpp
    core/Window.cpp
    graphics/GraphicsEngine.cpp
    genetics/GeneticsIntegration.cpp
)

# Link libraries
target_link_libraries(GeneticsGameEngine PRIVATE
    Core
    GraphicsEngine
    GeneticsIntegration
    Engine
    d3d12.lib
    dxgi.lib
    d3dcompiler.lib
    kernel32.lib
    user32.lib
    gdi32.lib
    winspool.lib
    shell32.lib
    ole32.lib
    oleaut32.lib
    uuid.lib
    comdlg32.lib
    advapi32.lib
)

# Set MSVC runtime library
if(MSVC)
    # Use MultiThreadedDLL for Release builds (dynamic CRT)
    # CMake automatically handles CRT linking - DO NOT manually link msvcrt.lib, ucrt.lib, or vcruntime.lib
    set_target_properties(GeneticsGameEngine PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    )
endif()

# Set compile options
set_target_properties(GeneticsGameEngine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Include directories
target_include_directories(GeneticsGameEngine PRIVATE
    core
    graphics
    genetics
    engine
    .
)

# Add compile definitions for Windows
target_compile_definitions(GeneticsGameEngine PRIVATE
    WIN32
    _WINDOWS
    UNICODE
    _UNICODE
    NOMINMAX
)

# Set Windows subsystem and entry point
set_target_properties(GeneticsGameEngine PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS OFF
    VS_WINRT_COMPONENT OFF
)

# Set Windows subsystem
if(MSVC)
    target_link_options(GeneticsGameEngine PRIVATE
        $<$<CONFIG:Release>:/SUBSYSTEM:CONSOLE>
        $<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>
    )
endif()

# Copy shader files to output directory (configuration-specific)
add_custom_command(TARGET GeneticsGameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/graphics/vertex.hlsl"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/vertex.hlsl"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/graphics/pixel.hlsl"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/pixel.hlsl"
    COMMENT "Copying shader files to output directory")
