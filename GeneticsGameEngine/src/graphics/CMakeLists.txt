# Graphics subsystem CMake configuration

# Add graphics source files
set(GRAPHICS_SOURCES
    GraphicsEngine.cpp
)

# Add graphics headers
set(GRAPHICS_HEADERS
    GraphicsEngine.h
)

# Create graphics library
add_library(GraphicsEngine STATIC
    ${GRAPHICS_SOURCES}
    ${GRAPHICS_HEADERS}
)

# Link dependencies
target_link_libraries(GraphicsEngine PRIVATE
    d3d12.lib
    dxgi.lib
    d3dcompiler.lib
)

# Set compile options
set_target_properties(GraphicsEngine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Include directories
target_include_directories(GraphicsEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add compile definitions for Windows
target_compile_definitions(GraphicsEngine PRIVATE
    WIN32
    _WINDOWS
    UNICODE
    _UNICODE
)
