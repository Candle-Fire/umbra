Include(FetchContent)
include(ExternalProject)

add_subdirectory(extern/dxmath)
add_subdirectory(extern/glm)
SET(SPDLOG_BUILD_PIC true)
add_subdirectory(extern/spdlog)
add_subdirectory(extern/dylib)
#add_subdirectory(extern/imgui)
add_subdirectory(extern/vulkna_memory_allocator)

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
