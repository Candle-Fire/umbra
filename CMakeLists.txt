cmake_minimum_required(VERSION 3.24)

include(deps.cmake)

if (MSVC)
    add_compile_options(-Zc:externC /permissive- /Zc:__cplusplus /Zc:inline /Zc:preprocessor /Zc:lambda /Zc:templateScope)
endif ()
if (MINGW)
    add_compile_options(-fpermissive)
endif ()

# Import some find files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(umbra)

set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

# Core engine
add_subdirectory(projs/shadow/shadow-engine)

# Runtime executable
add_subdirectory(projs/shadow/shadow-runtime)

add_subdirectory(projs/shadow/shadow-editor)

add_subdirectory(projs/test-game)