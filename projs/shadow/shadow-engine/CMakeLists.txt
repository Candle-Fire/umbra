find_package(Vulkan REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(imgui REQUIRED)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

FILE(GLOB_RECURSE SOURCES
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


add_executable(shadow-engine-test
        tests/test.main.cpp
        shadow-entity/src/tests/NodeContainer.test.cpp
        shadow-entity/src/tests/EntitySystem.test.cpp)

target_include_directories(shadow-engine-test PUBLIC ${IncludeDirs} PRIVATE ${SDL2_INCLUDE_DIRS})

target_link_libraries(shadow-engine-test
        PUBLIC Vulkan::Vulkan SDL2::SDL2 spdlog dylib imgui Catch2::Catch2 shadow-engine
        #-static -static-libgcc -static-libstdc++
        )

add_custom_command(TARGET shadow-engine-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:shadow-engine-test> $<TARGET_FILE_DIR:shadow-engine-test>
        COMMAND_EXPAND_LISTS
        )

#SET_TARGET_PROPERTIES(shadow-engine-test PROPERTIES LINK_FLAGS "/PROFILE")

include(CTest)
include(Catch)
catch_discover_tests(shadow-engine-test)