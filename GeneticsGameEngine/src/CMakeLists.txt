# Main source directory CMake configuration

# Add subdirectories
add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(genetics)

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
    d3d12.lib
    dxgi.lib
    d3dcompiler.lib
    # C++ runtime libraries
    msvcrt.lib
    vcruntime.lib
    ucrt.lib
)

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
        $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>
    )
endif()

# Copy shader files to output directory
add_custom_command(TARGET GeneticsGameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/graphics/vertex.hlsl"
        "${CMAKE_BINARY_DIR}/bin/vertex.hlsl"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/graphics/pixel.hlsl"
        "${CMAKE_BINARY_DIR}/bin/pixel.hlsl"
    COMMENT "Copying shader files to output directory")

# Set entry point
if(MSVC)
    target_link_options(GeneticsGameEngine PRIVATE
        $<$<CONFIG:Release>:/ENTRY:WinMain>
    )
endif()
