project(shadow)

include(FetchContent)

# External code
add_subdirectory(extern/dxmath)
add_subdirectory(extern/glm)

add_subdirectory(extern/dylib)
add_subdirectory(extern/vulkan_memory_allocator)
add_subdirectory(extern/Vulkan2D)

add_subdirectory(extern/catch2)

include(extern/SDL2/CMakeLists.txt)

SET(SPDLOG_BUILD_PIC true)
add_subdirectory(extern/spdlog)

# Core engine
add_subdirectory(shadow-engine)

# Runtime executable
add_subdirectory(shadow-runtime)

# Editor executable
add_subdirectory(shadow-editor)