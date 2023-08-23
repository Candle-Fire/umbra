Include(FetchContent)

if (WIN32)

    # ###############################################
    # Fetch SDL for the runtime
    # ###############################################
    FetchContent_Declare(
            SDL2
            URL https://github.com/libsdl-org/SDL/releases/download/release-2.26.5/SDL2-devel-2.26.5-VC.zip
    )
    FetchContent_MakeAvailable(SDL2)
    set(SDL2_DIR ${sdl2_SOURCE_DIR})
    list(PREPEND CMAKE_PREFIX_PATH "${sdl2_SOURCE_DIR}/cmake")

endif ()


# ###############################################
# Fetch Catch2 for the file format tests
# ###############################################
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.13.9 # or a later release
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/contrib")

# ###############################################
# Fetch GLM for the renderer
# ###############################################
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.2
)

FetchContent_GetProperties(glm)
if (NOT glm_POPULATED)
    FetchContent_Populate(glm)
    set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
    add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
endif ()

# ###############################################
# Fetch SpdLog for.. logging
# ###############################################
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.10.0
)

FetchContent_GetProperties(spdlog)
if (NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
    add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
endif ()

# ###############################################
# Imgui
# ###############################################
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG d2291df55190e2f070af2635863f47a96d378a52
)
FetchContent_MakeAvailable(imgui)

# ###############################################
# dynlib - DLL loading libary
# ###############################################
FetchContent_Declare(
        dylib
        GIT_REPOSITORY "https://github.com/martin-olivier/dylib"
        GIT_TAG "v2.1.0"
)
FetchContent_MakeAvailable(dylib)

# ###############################################
# Vulkan Memory Allocator
# ###############################################
FetchContent_Declare(
        VMA
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG v3.0.1
)
FetchContent_MakeAvailable(VMA)

# ###############################################
# Vulkan 2D rendering library
# ###############################################
FetchContent_Declare(
        Vulkan2D
        GIT_REPOSITORY "https://github.com/dpeter99/Vulkan2D.git"
        GIT_TAG 0b99f3d056c67cab36b49323492717537de27b8f
)
FetchContent_MakeAvailable(Vulkan2D)