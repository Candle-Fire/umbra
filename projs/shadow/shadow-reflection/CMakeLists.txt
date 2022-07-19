set(CMAKE_CXX_STANDARD 20)
include_directories(${SDL2_INCLUDE_DIRS})

FILE(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

add_library(shadow-reflect ${SOURCES})