cmake_minimum_required(VERSION 3.24)
project(sample)

# Add SDL
add_subdirectory(ext/SDL EXCLUDE_FROM_ALL)

# Include ImGui headers
include_directories(
    ext/SDL/include
    ext/imgui
    ext/imgui/backends
)

# Create ImGui library (using SDL and software rendering)
add_library(imgui 
    ext/imgui/imgui.cpp 
    ext/imgui/imgui_draw.cpp
    ext/imgui/imgui_demo.cpp
    ext/imgui/imgui_tables.cpp
    ext/imgui/imgui_widgets.cpp
    ext/imgui/backends/imgui_impl_sdl3.cpp  # Only SDL backend
    ext/imgui/backends/imgui_impl_sdlrenderer3.cpp  # Only SDL backend
)

target_link_libraries(imgui PRIVATE SDL3::SDL3)

add_executable(sample main.cpp window.cpp)

target_link_libraries(sample PRIVATE SDL3::SDL3 imgui)
