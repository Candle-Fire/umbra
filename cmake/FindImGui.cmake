set(CMAKE_CXX_STANDARD 20)
find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

SET(imgui_SOURCE_DIR ${CMAKE_SOURCE_DIR}/extern/imgui)

FILE(GLOB_RECURSE SOURCES ${imgui_SOURCE_DIR}/*.cpp)
FILE(GLOB_RECURSE HEADERS ${imgui_SOURCE_DIR}/*.h)

FILE(GLOB_RECURSE HEADERS ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.h)
FILE(GLOB_RECURSE SOURCES ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp)

add_library(ImGui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
#        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
#        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

target_include_directories(ImGui
        PRIVATE
        ${SDL2_INCLUDE_DIRS}
        PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(ImGui
#        PRIVATE SDL2::SDL2 Vulkan::Vulkan
)

target_compile_options(ImGui PRIVATE -fPIC)