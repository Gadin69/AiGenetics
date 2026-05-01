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

# Set entry point
if(MSVC)
    target_link_options(GeneticsGameEngine PRIVATE
        $<$<CONFIG:Release>:/ENTRY:WinMainCRTStartup>
    )
endif()
