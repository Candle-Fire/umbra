#find_package(Vulkan REQUIRED)
#find_package(DirectXMath REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
#find_package(glm REQUIRED)
find_package(ImGui REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(shadow-engine SHARED)
add_library(shadow::engine ALIAS shadow-engine)

add_executable(shadow-engine-tests)
target_link_libraries(shadow-engine-tests
        Catch2::Catch2
        shadow::engine
)

add_subdirectory(assets)
add_subdirectory(core)
add_subdirectory(entity)
add_subdirectory(math)
add_subdirectory(platforms)
add_subdirectory(reflection)
add_subdirectory(renderer)
add_subdirectory(utility)
add_subdirectory(tests)

target_link_libraries(shadow-engine
        PUBLIC
        DirectXMath
        SDL2::SDL2
        glm::glm
        spdlog::spdlog
        dylib
        #        ImGui
        GPUOpen::VulkanMemoryAllocator
        Vulkan::Vulkan
)

target_compile_definitions(shadow-engine
        PRIVATE
        EXPORTING_SH_ENGINE
)

if (TRUE)

    target_sources(shadow-engine PRIVATE
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
    )

    target_include_directories(shadow-engine
            PUBLIC
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends
    )

    target_compile_definitions(shadow-engine
            PRIVATE
            IMGUI_USER_CONFIG="shadow/imguiConf.h"
    )

endif ()



