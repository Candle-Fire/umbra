cmake_minimum_required(VERSION 3.26)

#set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)

#if (MSVC)
#    add_compile_options(-Zc:externC /permissive- /Zc:__cplusplus /Zc:inline /Zc:preprocessor /Zc:lambda /Zc:templateScope)
#endif ()
if (MINGW OR LINUX)
    #add_compile_options(-fpermissive)
endif ()

# Import some find files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(umbra)

set(BUILD_DIR "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/")
set(BUILD_DIR_CMAKE "${BUILD_DIR}/${CMAKE_GENERATOR_PLATFORM}}/")
include(shadow-modules.cmake)

set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

add_subdirectory(projs/shadow)

add_subdirectory(projs/test-game)