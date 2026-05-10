# Main source directory CMake configuration

# Add third-party libraries
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../third_party ${CMAKE_BINARY_DIR}/third_party)

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
    Neural
    UI
    imgui
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
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui/backends
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

# Copy ALL shader files to output directory automatically
# This ensures shaders are always in sync with the executable

# Define the actual output directory (matches root CMakeLists.txt setting)
set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

# Step 1: Copy graphics shaders from src/graphics/
file(GLOB GRAPHICS_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/graphics/*.hlsl")
foreach(shader ${GRAPHICS_SHADERS})
    get_filename_component(shader_name ${shader} NAME)
    add_custom_command(TARGET GeneticsGameEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_OUTPUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${shader}"
            "${SHADER_OUTPUT_DIR}/${shader_name}"
        COMMENT "Copying shader: ${shader_name}"
    )
endforeach()

# Step 2: Copy engine shaders from src/shaders/
file(GLOB ENGINE_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.hlsl")
foreach(shader ${ENGINE_SHADERS})
    get_filename_component(shader_name ${shader} NAME)
    add_custom_command(TARGET GeneticsGameEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_OUTPUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${shader}"
            "${SHADER_OUTPUT_DIR}/${shader_name}"
        COMMENT "Copying shader: ${shader_name}"
    )
endforeach()

# Step 3: Copy root-level shaders from shaders/ (if any)
file(GLOB ROOT_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/*.hlsl")
foreach(shader ${ROOT_SHADERS})
    get_filename_component(shader_name ${shader} NAME)
    add_custom_command(TARGET GeneticsGameEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_OUTPUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${shader}"
            "${SHADER_OUTPUT_DIR}/${shader_name}"
        COMMENT "Copying shader: ${shader_name}"
    )
endforeach()
