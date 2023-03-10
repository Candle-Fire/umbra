find_package(Vulkan REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(imgui REQUIRED)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_subdirectory(shadow-assets)

FILE(GLOB_RECURSE SOURCES
        core/src/**/*.cpp
        shadow-assets/src/*.cpp
        shadow-entity/src/*.cpp
        shadow-renderer/src/*.cpp
        shadow-reflection/src/*.cpp
        shadow-utility/src/*.cpp
        shadow-math/src/*.cpp
        )
list(FILTER SOURCES EXCLUDE REGEX ".*.test.cpp$")

SET(IncludeDirs
        core/inc
        shadow-assets/inc
        shadow-entity/inc
        shadow-renderer/inc
        shadow-reflection/inc
        shadow-utility/inc
        shadow-math/inc
        )


add_library(shadow-engine SHARED ${SOURCES} $<TARGET_OBJECTS:imgui>)

target_include_directories(shadow-engine
        PRIVATE ${SDL2_INCLUDE_DIRS}
        PUBLIC ${IncludeDirs}
        ${glm_SOURCE_DIR}
        INTERFACE
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends)

target_link_libraries(shadow-engine
        PUBLIC Vulkan::Vulkan SDL2::SDL2 spdlog dylib imgui
        PRIVATE -static-libgcc -static-libstdc++
        )

target_compile_definitions(shadow-engine PRIVATE "EXPORTING_SH_ENGINE")

target_link_options(shadow-engine PUBLIC -Wl,--export-all-symbols)


add_executable(tests ${TEST_SOURCES} ${SOURCES} $<TARGET_OBJECTS:imgui>)
target_link_libraries(tests PRIVATE Catch2::Catch2 PUBLIC Vulkan::Vulkan SDL2::SDL2 spdlog dylib imgui)
target_include_directories(tests
        PUBLIC ${IncludeDirs}
        PRIVATE ${SDL2_INCLUDE_DIRS} ${glm_SOURCE_DIR}
        INTERFACE
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        )
target_compile_definitions(tests PRIVATE "EXPORTING_SH_ENGINE")


# Enable testing on the executable
include(CTest)
include(Catch)
catch_discover_tests(tests)