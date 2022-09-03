find_package(SDL2 CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 20)

FILE(GLOB_RECURSE SOURCES src/*.cpp)
FILE(GLOB_RECURSE HEADERS src/*.h)

add_library(test-game SHARED ${SOURCES})

# ../shadow/shadow-engine/inc
target_include_directories(test-game PRIVATE ${SDL2_INCLUDE_DIRS} PUBLIC inc)
target_link_libraries(test-game PUBLIC shadow-engine)


add_custom_command(TARGET test-game
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/vlkx-resources
        )