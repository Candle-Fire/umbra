find_package(SDL2 CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 20)

FILE(GLOB_RECURSE SOURCES src/*.cpp)
FILE(GLOB_RECURSE HEADERS src/*.h)

set(GENERATE_COMMAND P:/_Projects/umbra/umbra/bin/Debug/MSBuild/net7.0/win-x64/shadow-header-tool.exe)

execute_process(
        COMMAND "P:/_Projects/umbra/umbra/bin/Debug/MSBuild/net7.0/win-x64/shadow-header-tool.exe" "dependencies -nb -p ${TARGET_NAME}"
        OUTPUT_VARIABLE DEPENDENCIES
        RESULT_VARIABLE RETURN_VALUE
)
if (NOT RETURN_VALUE EQUAL 0)
    message(FATAL_ERROR "Failed to get the dependencies")
endif ()

set(SH_GENERATED ${PROJECT_SOURCE_DIR}/generated.cpp)

add_custom_command(
        COMMAND "P:/_Projects/umbra/umbra/bin/Debug/MSBuild/net7.0/win-x64/shadow-header-tool.exe" "generate --project ${TARGET_NAME} -o ${SH_GENERATED}"
        DEPENDS ${DEPENDENCIES}
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

