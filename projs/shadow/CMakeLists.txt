project(shadow)

# External code
add_subdirectory(extern/dxmath)
add_subdirectory(extern/glm)
SET(SPDLOG_BUILD_PIC true)
add_subdirectory(extern/spdlog)
add_subdirectory(extern/dylib)
add_subdirectory(extern/vulkan_memory_allocator)
add_subdirectory(extern/cathc2)

# Core engine
add_subdirectory(shadow-engine)

# Runtime executable
add_subdirectory(shadow-runtime)

# Editor executable
add_subdirectory(shadow-editor)