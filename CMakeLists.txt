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

# Import some find files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(umbra)

# Common utilities
add_subdirectory(projs/shadow/shadow-utility)

# Asset management
add_subdirectory(projs/shadow/shadow-file-format)

# Reflection
add_subdirectory(projs/shadow/shadow-reflection)

# Core engine
add_subdirectory(projs/shadow/shadow-engine)

# Runtime executable
add_subdirectory(projs/shadow/shadow-runtime)