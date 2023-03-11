find_package(Vulkan REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(imgui REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

FILE(GLOB_RECURSE SOURCES
        core/src/*.cpp
        shadow-entity/src/*.cpp
        shadow-renderer/src/*.cpp
        shadow-reflection/src/*.cpp
        shadow-utility/src/*.cpp
        )
FILE(GLOB_RECURSE HEADERS
        core/inc/*.h
        shadow-entity/inc/*.h
        shadow-renderer/inc/*.h
        shadow-reflection/inc/*.h
        shadow-utility/inc/*.h
        )

add_library(shadow-engine SHARED ${SOURCES} $<TARGET_OBJECTS:imgui>)

target_include_directories(shadow-engine
        PRIVATE ${SDL2_INCLUDE_DIRS}
        PUBLIC
            core/inc
            shadow-entity/inc
            shadow-renderer/inc
            shadow-reflection/inc
            shadow-utility/inc
        ${glm_SOURCE_DIR}
        INTERFACE
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends)

target_link_libraries(shadow-engine
        PUBLIC Vulkan::Vulkan SDL2::SDL2 spdlog dylib imgui
)
target_compile_definitions(shadow-engine PRIVATE "EXPORTING_SH_ENGINE")

target_link_options(shadow-engine PUBLIC -Wl,--export-all-symbols)

