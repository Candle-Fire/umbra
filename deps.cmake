Include(FetchContent)
include(ExternalProject)


add_subdirectory(projs/extern/fmt)

# ###############################################
# Fetch Catch2 for the file format tests
# ###############################################
#FetchContent_Declare(
#        Catch2
#        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
#        GIT_TAG v2.13.9 # or a later release
#)
#FetchContent_MakeAvailable(Catch2)
#list(APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/contrib")
