set(CMAKE_CXX_STANDARD 20)
find_package(SDL2 CONFIG REQUIRED)

FILE(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

add_executable(shadow-runtime ${SOURCES})

target_include_directories(shadow-runtime PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(shadow-runtime PRIVATE SDL2::SDL2main shadow-engine)

add_custom_command(TARGET shadow-runtime
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${SDL2_BINDIR}/SDL2.dll C:/Users/dpete/Documents/programing/_Projects/umbra/umbra/cmake-build-debug/projs/shadow/shadow-runtime/.
        )