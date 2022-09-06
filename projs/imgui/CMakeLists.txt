set(CMAKE_CXX_STANDARD 20)
find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

FILE(GLOB_RECURSE SOURCES *.cpp)
FILE(GLOB_RECURSE HEADERS *.h)

add_library(imgui ${SOURCES})

target_include_directories(imgui PRIVATE ${SDL2_INCLUDE_DIRS} PUBLIC .)
target_link_libraries(imgui PRIVATE ${SDL2_LIBRARIES} Vulkan::Vulkan)
