# Genetics subsystem CMake configuration

# Add genetics source files
set(GENETICS_SOURCES
    GeneticsIntegration.cpp
)

# Add genetics headers
set(GENETICS_HEADERS
    GeneticsIntegration.h
)

# Create genetics library
add_library(GeneticsIntegration STATIC
    ${GENETICS_SOURCES}
    ${GENETICS_HEADERS}
)

# Set compile options
set_target_properties(GeneticsIntegration PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Include directories
target_include_directories(GeneticsIntegration PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add compile definitions for Windows
target_compile_definitions(GeneticsIntegration PRIVATE
    WIN32
    _WINDOWS
    UNICODE
    _UNICODE
)
