project(shadow)

# External code
add_subdirectory(extern/dxmath)
add_subdirectory(extern/glm)
SET(SPDLOG_BUILD_PIC true)
add_subdirectory(extern/spdlog)
add_subdirectory(extern/dylib)
add_subdirectory(extern/vulkan_memory_allocator)
add_subdirectory(extern/cathc2)

if (WIN32)
    # Fetch SDL for the runtime
    FetchContent_Declare(
            SDL2
            URL https://www.libsdl.org/release/SDL2-devel-2.26.0-VC.zip
    )
    FetchContent_MakeAvailable(SDL2)
    set(SDL2_DIR ${sdl2_SOURCE_DIR})
    list(PREPEND CMAKE_PREFIX_PATH "${sdl2_SOURCE_DIR}/cmake")

endif ()

# Core engine
add_subdirectory(shadow-engine)

# Runtime executable
add_subdirectory(shadow-runtime)

# Editor executable
add_subdirectory(shadow-editor)