find_package(Vulkan REQUIRED)
find_package(DirectXMath REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(ImGui REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(shadow-engine SHARED
        platforms/linux/inc/sal.h)

FILE(GLOB_RECURSE SOURCES
        core/src/**/*.cpp
        core/src/*.cpp
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
        platforms/inc
)

# ##################################################
# ################# PLATFORMS ######################
# ##################################################
# Windows
if (WIN32)
    FILE(GLOB_RECURSE WIN_PLATFORM_SOURCES
            platforms/windows/src/*.cpp
    )
    LIST(APPEND SOURCES ${WIN_PLATFORM_SOURCES})
endif ()
# Linux
if (UNIX AND NOT APPLE)
    FILE(GLOB_RECURSE LINUX_PLATFORM_SOURCES
            platforms/linux/src/*.cpp
    )
    LIST(APPEND SOURCES ${LINUX_PLATFORM_SOURCES})
    list(APPEND IncludeDirs platforms/linux/inc)
endif ()

# ##################################################
# DirectX 12
# ##################################################
if (WIN32 AND DX12)
    find_package(D3D12 REQUIRED)
    add_dx12(shadow-engine)
endif ()

target_sources(shadow-engine PRIVATE ${SOURCES} $<TARGET_OBJECTS:imgui>
        core/src/compilation-test.cpp)

target_include_directories(shadow-engine
        PRIVATE ${SDL2_INCLUDE_DIRS}
        ${DirectXMath_INCLUDE_DIRS}
        ${D3D12_INCLUDE_DIRS}
        PUBLIC ${IncludeDirs}
        ${glm_SOURCE_DIR}
        INTERFACE
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        PUBLIC ${PROJECT_SOURCE_DIR}/extern/spdlog/include/)

#add_dependencies(shadow-engine spdlog)

target_link_libraries(shadow-engine
        PUBLIC Vulkan::Vulkan SDL2::SDL2 dylib imgui
        PRIVATE -static-libgcc -static-libstdc++
)

target_compile_definitions(shadow-engine PRIVATE "EXPORTING_SH_ENGINE")

if (WIN32 AND MINGW)
    target_link_options(shadow-engine PUBLIC -Wl,--export-all-symbols)
endif ()

add_executable(tests ${TEST_SOURCES} ${SOURCES} $<TARGET_OBJECTS:imgui>)
target_link_libraries(tests PRIVATE Catch2::Catch2 PUBLIC Vulkan::Vulkan SDL2::SDL2 dylib imgui)
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