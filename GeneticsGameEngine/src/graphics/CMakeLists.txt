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

# Set MSVC runtime library
if(MSVC)
    set_target_properties(GraphicsEngine PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    )
endif()

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

# Enable /showIncludes to debug circular dependencies
if(MSVC)
    target_compile_options(GraphicsEngine PRIVATE "/showIncludes")
endif()

# Include directories
target_include_directories(GraphicsEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link procedural library
target_link_libraries(GraphicsEngine PRIVATE
    Procedural
)

# Add compile definitions for Windows
target_compile_definitions(GraphicsEngine PRIVATE
    WIN32
    _WINDOWS
    UNICODE
    _UNICODE
)
