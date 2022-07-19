set(CMAKE_CXX_STANDARD 20)
find_package(SDL2 REQUIRED)

add_executable(shadow-runtime src/main.cpp)

target_include_directories(shadow-runtime PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(shadow-runtime PRIVATE SDL2::Main shadow-engine)
