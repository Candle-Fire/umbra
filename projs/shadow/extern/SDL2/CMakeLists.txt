

# We need to fetch SDL2 for the runtime on Windows
if (WIN32)
    # Fetch SDL for the runtime
    FetchContent_Declare(
            SDL2
            URL https://www.libsdl.org/release/SDL2-devel-2.30.2-VC.zip
    )
    FetchContent_MakeAvailable(SDL2)
    set(SDL2_DIR ${sdl2_SOURCE_DIR})
    list(APPEND CMAKE_PREFIX_PATH "${sdl2_SOURCE_DIR}/cmake")

endif ()