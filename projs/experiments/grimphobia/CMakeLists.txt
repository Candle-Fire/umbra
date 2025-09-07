
set(CMAKE_CXX_STANDARD 26)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

SET(imgui_SOURCE_DIR ${CMAKE_SOURCE_DIR}/projs/experiments/grimphobia/extern/ImGUI)

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(extern/SDL3 EXCLUDE_FROM_ALL)
add_subdirectory(extern/SDL3_image EXCLUDE_FROM_ALL)

FILE(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)

add_executable(grimphobia)

target_sources(grimphobia
        PRIVATE
        ${SOURCES}

        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlgpu3.cpp
)

target_include_directories(grimphobia
        PRIVATE
        src/
        ${imgui_SOURCE_DIR}
)

# Link to the actual SDL3 library.
target_link_libraries(grimphobia PRIVATE SDL3_image::SDL3_image SDL3::SDL3)
