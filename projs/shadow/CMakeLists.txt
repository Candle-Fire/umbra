project(shadow)

include(FetchContent)

# External code
add_subdirectory(extern/dxmath)
add_subdirectory(extern/glm)
SET(SPDLOG_BUILD_PIC true)
add_subdirectory(extern/spdlog)
add_subdirectory(extern/dylib)
add_subdirectory(extern/vulkan_memory_allocator)
add_subdirectory(extern/catch2)
include(extern/SDL2/CMakeLists.txt)

# ###############################################
# Vulkan 2D rendering library
# ###############################################
FetchContent_Declare(
        Vulkan2D
        GIT_REPOSITORY "https://github.com/dpeter99/Vulkan2D.git"
        GIT_TAG 0b99f3d056c67cab36b49323492717537de27b8f
)
FetchContent_MakeAvailable(Vulkan2D)

# Core engine
add_subdirectory(shadow-engine)

# Runtime executable
add_subdirectory(shadow-runtime)

# Editor executable
add_subdirectory(shadow-editor)