# Main source directory CMake configuration

# Add subdirectories
add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(genetics)
add_subdirectory(physics)
add_subdirectory(audio)

# Create executable
add_executable(GeneticsGameEngine
    core/main.cpp
    graphics/GraphicsEngine.cpp
    core/Window.cpp
)

# Link libraries
target_link_libraries(GeneticsGameEngine PRIVATE
    Core
    GraphicsEngine
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
)

# Add compile definitions for Windows
target_compile_definitions(GeneticsGameEngine PRIVATE
    WIN32
    _WINDOWS
    UNICODE
    _UNICODE
)
