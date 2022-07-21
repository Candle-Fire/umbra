cmake_minimum_required(VERSION 3.22)

Include(FetchContent)

# Fetch SDL for the runtime
FetchContent_Declare(
        SDL2
        URL     https://www.libsdl.org/release/SDL2-devel-2.0.22-VC.zip
)
FetchContent_MakeAvailable(SDL2)
set(SDL2_PATH ${sdl2_SOURCE_DIR})

# Fetch Catch2 for the file format tests
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v2.13.9 # or a later release
)

FetchContent_MakeAvailable(Catch2)

# Fetch GLM for the renderer
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG		   0.9.9.2
)

FetchContent_GetProperties(glm)
if(NOT glm_POPULATED)
    FetchContent_Populate(glm)
    set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
    add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
endif()

# Fetch SpdLog for.. loggin
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG  v1.10.0
)

FetchContent_GetProperties(spdlog)
if(NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
    add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
endif()


# Import some find files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(umbra)

# Fetch ImGui for the application
add_subdirectory(projs/imgui)

# Common utilities
add_subdirectory(projs/shadow/shadow-utility)

# Asset management
add_subdirectory(projs/shadow/shadow-file-format)

# Reflection
add_subdirectory(projs/shadow/shadow-reflection)

# Core engine
add_subdirectory(projs/shadow/shadow-engine)

# Renderer
add_subdirectory(projs/shadow/shadow-renderer)

# Runtime executable
add_subdirectory(projs/shadow/shadow-runtime)