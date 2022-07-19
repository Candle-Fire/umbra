find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

set(CMAKE_CXX_STANDARD 20)

FILE(GLOB_RECURSE SOURCES src/*.cpp)
FILE(GLOB_RECURSE HEADERS src/*.h)

add_library(shadow-engine ${SOURCES})

target_include_directories(shadow-engine PRIVATE ${SDL2_INCLUDE_DIRS} PUBLIC ${HEADERS})
target_link_libraries(shadow-engine PRIVATE Vulkan::Vulkan PUBLIC SDL2::Core shadow-reflect shadow-asset)