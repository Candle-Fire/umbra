include(FetchContent)

FetchContent_Declare(
        sdl3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL
        GIT_TAG 4bebbec9d442a63f592cdb310b1172f535f72e7d
)
FetchContent_MakeAvailable(sdl3)
