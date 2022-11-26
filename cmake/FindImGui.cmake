set(CMAKE_CXX_STANDARD 20)
find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

FILE(GLOB_RECURSE SOURCES ${imgui_SOURCE_DIR}/*.cpp)
FILE(GLOB_RECURSE HEADERS ${imgui_SOURCE_DIR}/*.h)

FILE(GLOB_RECURSE HEADERS ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.h)
FILE(GLOB_RECURSE SOURCES ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp)

add_library(imgui OBJECT
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
        )

target_include_directories(imgui
        PUBLIC
        ${SDL2_INCLUDE_DIRS}
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        )
target_link_libraries(imgui PRIVATE SDL2::SDL2 Vulkan::Vulkan)