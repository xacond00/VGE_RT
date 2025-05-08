cmake_minimum_required(VERSION 3.24)
project(vgert)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # Optional: disables compiler-specific extensions
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
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
    src/acc_kdtree.cpp
    src/window.cpp
)
target_compile_options(vgert PRIVATE -O3)
target_link_libraries(vgert PRIVATE SDL3::SDL3 imgui)
