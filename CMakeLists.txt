cmake_minimum_required(VERSION 3.24)

Include(FetchContent)

# Fetch SDL for the runtime
FetchContent_Declare(
        SDL2
        URL     https://www.libsdl.org/release/SDL2-devel-2.24.0-VC.zip
)
FetchContent_MakeAvailable(SDL2)
set(SDL2_DIR ${sdl2_SOURCE_DIR})
list(PREPEND CMAKE_PREFIX_PATH "${sdl2_SOURCE_DIR}/cmake")

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

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG 71a0701920dbc83155f718182f01132d1ec2d51e
)

FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
        dylib
        GIT_REPOSITORY "https://github.com/martin-olivier/dylib"
        GIT_TAG        "v2.1.0"
)

FetchContent_MakeAvailable(dylib)

# Import some find files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(umbra)

set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

# Core engine
add_subdirectory(projs/shadow/shadow-engine)

# Runtime executable
add_subdirectory(projs/shadow/shadow-runtime)

add_subdirectory(projs/test-game)