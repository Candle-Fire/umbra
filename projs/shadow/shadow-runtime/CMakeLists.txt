set(CMAKE_CXX_STANDARD 20)
find_package(SDL2 CONFIG REQUIRED)

FILE(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

add_executable(shadow-runtime ${SOURCES})

target_include_directories(shadow-runtime PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(shadow-runtime PRIVATE SDL2::SDL2main PUBLIC shadow-engine)

add_custom_command(TARGET shadow-runtime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:shadow-runtime> $<TARGET_FILE_DIR:shadow-runtime>
        COMMAND_EXPAND_LISTS
)

add_custom_command(TARGET shadow-runtime
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/vlkx-resources ${CMAKE_CURRENT_BINARY_DIR}/vlkx-resources
        )