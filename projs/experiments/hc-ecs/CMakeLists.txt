set(CMAKE_CXX_STANDARD 23)

add_subdirectory(../../shadow/extern/dxmath extern/dxmath)
add_subdirectory(../../shadow/extern/glm extern/glm)

add_executable(experiment-hc-ecs)
add_executable(experiment::hc-ecs ALIAS experiment-hc-ecs)


FILE(GLOB_RECURSE SOURCES
        src/*.cpp
)

target_sources(experiment-hc-ecs PUBLIC ${SOURCES})
target_include_directories(experiment-hc-ecs PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)

target_link_libraries(experiment-hc-ecs
        PRIVATE fmt::fmt-header-only
        PRIVATE spdlog::spdlog_header_only
        Catch2::Catch2
        DirectXMath
        glm::glm
)

FILE(GLOB_RECURSE SOURCES_TESTS
        src/*.cpp
        tests/*.cpp
)

add_executable(experiment-hc-ecs-tests)
target_link_libraries(experiment-hc-ecs-tests
        PRIVATE fmt::fmt-header-only
        PRIVATE spdlog::spdlog_header_only
        Catch2::Catch2WithMain
        DirectXMath
        glm::glm
)
target_compile_options(experiment-hc-ecs-tests
    PUBLIC -fconcepts-diagnostics-depth=4
)
target_sources(experiment-hc-ecs-tests PUBLIC ${SOURCES_TESTS})
target_include_directories(experiment-hc-ecs-tests PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)