set(CMAKE_CXX_STANDARD 20)
find_package(SDL2 CONFIG REQUIRED)

add_executable(shadow-editor)
add_executable(shadow::editor ALIAS shadow-editor)


FILE(GLOB_RECURSE SOURCES
        src/*.cpp
)

target_sources(shadow-editor PUBLIC ${SOURCES})

target_include_directories(shadow-editor
        PUBLIC inc/
        PRIVATE ${SDL2_INCLUDE_DIRS})

target_link_libraries(shadow-editor
        PRIVATE SDL2::SDL2main
        PUBLIC shadow-engine
)

target_link_options(shadow-editor
        PRIVATE -Wl,--export-dynamic
)

add_custom_command(TARGET shadow-editor
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/vlkx-resources ${CMAKE_CURRENT_BINARY_DIR}/vlkx-resources
)

if (WIN32)
    add_custom_command(TARGET shadow-editor
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:shadow-runtime> $<TARGET_FILE_DIR:shadow-editor>
            COMMAND_EXPAND_LISTS
    )
endif ()