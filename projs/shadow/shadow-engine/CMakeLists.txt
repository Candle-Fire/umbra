find_package(Vulkan REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(imgui REQUIRED)

set(CMAKE_CXX_STANDARD 20)

FILE(GLOB_RECURSE SOURCES
        core/src/*.cpp
        shadow-renderer/src/*.cpp
        )
FILE(GLOB_RECURSE HEADERS
        core/inc/*.h
        shadow-renderer/inc/*.h
        )

add_library(shadow-engine ${SOURCES})

target_include_directories(shadow-engine
        PRIVATE ${SDL2_INCLUDE_DIRS}
        PUBLIC
            core/inc
            shadow-renderer/inc
        ${glm_SOURCE_DIR})

target_link_libraries(shadow-engine PRIVATE Vulkan::Vulkan PUBLIC SDL2::SDL2 spdlog dylib PUBLIC imgui)

target_compile_definitions(shadow-engine PRIVATE "EXPORTING_SH_ENGINE")
