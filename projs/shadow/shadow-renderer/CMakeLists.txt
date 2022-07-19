set(CMAKE_CXX_STANDARD 20)
find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

FILE(GLOB_RECURSE SOURCES src/*.cpp inc/*.h)

add_library(renderer ${SOURCES})

target_include_directories(renderer PRIVATE ${SDL2_INCLUDE_DIRS} PUBLIC inc)
target_link_libraries(renderer PRIVATE SDL2::Main Vulkan::Vulkan)
