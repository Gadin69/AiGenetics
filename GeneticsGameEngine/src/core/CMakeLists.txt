# Core subsystem CMake configuration

# Add core source files
set(CORE_SOURCES
    main.cpp
    Window.cpp
)

# Add core headers
set(CORE_HEADERS
    Window.h
)

# Create core library
add_library(Core STATIC
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

# Set MSVC runtime library
if(MSVC)
    set_target_properties(Core PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    )
endif()

# Set compile options
set_target_properties(Core PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Include directories
target_include_directories(Core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add compile definitions for Windows
target_compile_definitions(Core PRIVATE
    WIN32
    _WINDOWS
    UNICODE
    _UNICODE
)
