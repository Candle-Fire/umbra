set(CMAKE_CXX_STANDARD 20)
find_package(SDL2 CONFIG REQUIRED)

add_executable(shadow-runtime)

FILE(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# ##################################################
# DirectX 12
# ##################################################
if (WIN32 AND DX12)
    find_package(D3D12 REQUIRED)
    add_dx12(shadow-editor)
    LIST(APPEND SOURCES ../platforms/dx12/dx12_agility.cpp)
endif ()

target_sources(shadow-editor PRIVATE ${SOURCES})

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

#SET_TARGET_PROPERTIES(shadow-runtime PROPERTIES LINK_FLAGS "/PROFILE")