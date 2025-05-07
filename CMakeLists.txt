cmake_minimum_required(VERSION 3.24)
project(vgert)

# Add SDL
add_subdirectory(ext/SDL EXCLUDE_FROM_ALL)

# Include directories
include_directories(
    ext/SDL/include
    ext/imgui
    ext/imgui/backends
    src  # Add this if you have headers in src/
)

# Create ImGui library
add_library(imgui 
    ext/imgui/imgui.cpp 
    ext/imgui/imgui_draw.cpp
    ext/imgui/imgui_demo.cpp
    ext/imgui/imgui_tables.cpp
    ext/imgui/imgui_widgets.cpp
    ext/imgui/backends/imgui_impl_sdl3.cpp
    ext/imgui/backends/imgui_impl_sdlrenderer3.cpp
)

target_link_libraries(imgui PRIVATE SDL3::SDL3)

# Update source file paths
add_executable(vgert
    src/main.cpp
    src/acc_bvh.cpp
    src/acc_bih.cpp
    src/window.cpp
)

target_link_libraries(vgert PRIVATE SDL3::SDL3 imgui)
