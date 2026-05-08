# UI Module

add_library(UI
    ImGuiRenderer.cpp
    ImGuiRenderer.h
)

target_include_directories(UI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(UI PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/imgui
    ${CMAKE_SOURCE_DIR}/third_party/imgui/backends
)

target_link_libraries(UI PUBLIC
    imgui
    d3d12
    dxgi
)
