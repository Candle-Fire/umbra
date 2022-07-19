set(CMAKE_CXX_STANDARD 20)
find_package(SDL2 REQUIRED)

FILE(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

add_executable(shadow-runtime ${SOURCES})

target_include_directories(shadow-runtime PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(shadow-runtime PRIVATE SDL2::Main shadow-engine)
