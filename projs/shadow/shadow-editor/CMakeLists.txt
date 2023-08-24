set(CMAKE_CXX_STANDARD 20)
find_package(SDL2 CONFIG REQUIRED)

add_executable(shadow-editor)

FILE(GLOB_RECURSE SOURCES src/*.cpp inc/*.h)

# ##################################################
# DirectX 12
# ##################################################
if (WIN32 AND DX12)
    find_package(D3D12 REQUIRED)
    add_dx12(shadow-editor)
    LIST(APPEND SOURCES ../platforms/dx12/dx12_agility.cpp)
endif ()

target_sources(shadow-editor PRIVATE ${SOURCES})

target_include_directories(shadow-editor
        PUBLIC inc/
        PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(shadow-editor PRIVATE SDL2::SDL2main PUBLIC shadow-engine)

add_custom_command(TARGET shadow-editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:shadow-runtime> $<TARGET_FILE_DIR:shadow-editor>
        COMMAND_EXPAND_LISTS
)

if (MSVC)
    add_custom_command(TARGET shadow-editor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:shadow-engine> $<TARGET_FILE_DIR:shadow-editor>
            COMMAND_EXPAND_LISTS
            COMMENT "Copy over shadow engine pdb file. This only copies the this one dependency and will need more work"
    )
endif ()

add_custom_command(TARGET shadow-editor
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/vlkx-resources ${CMAKE_CURRENT_BINARY_DIR}/vlkx-resources
)