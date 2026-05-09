# Dear ImGui library
add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_dx12.cpp
    imgui/backends/imgui_impl_win32.cpp
)

target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)

# Link required Windows libraries
target_link_libraries(imgui PUBLIC
    d3d12
    dxgi
)

# Set compile options
target_compile_options(imgui PRIVATE
    /w  # Disable all warnings for third-party code
)
