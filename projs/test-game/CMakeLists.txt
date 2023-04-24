find_package(SDL2 CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 20)

FILE(GLOB_RECURSE SOURCES src/*.cpp)
FILE(GLOB_RECURSE HEADERS src/*.h)

set(GENERATE_COMMAND P:/_Projects/umbra/umbra/bin/Debug/MSBuild/net7.0/win-x64/shadow-header-tool.exe)

set(SH_GENERATED ${CMAKE_CURRENT_SOURCE_DIR}/gen/generated.cpp)

add_custom_command(
        COMMAND "P:/_Projects/umbra/umbra/bin/Debug/MSBuild/net7.0/win-x64/shadow-header-tool.exe" generate -p test-game -o "${SH_GENERATED}"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        DEPENDS ${SOURCES} ${HEADERS}
        OUTPUT ${SH_GENERATED}
        COMMENT "Reflection information generator"
)

add_library(test-game SHARED
        ${SOURCES}
        ${SH_GENERATED}
        )

target_include_directories(test-game PRIVATE PUBLIC inc)
target_link_libraries(test-game PUBLIC shadow-engine)


add_custom_command(TARGET test-game
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
        )

