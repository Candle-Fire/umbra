set(CMAKE_CXX_STANDARD 20)

add_executable(experiment-ecs)
add_executable(experiment::ecs ALIAS experiment-ecs)


FILE(GLOB_RECURSE SOURCES
        src/*.cpp
)

target_sources(experiment-ecs PUBLIC ${SOURCES})

target_link_libraries(experiment-ecs
        PRIVATE fmt::fmt-header-only
)


FILE(GLOB_RECURSE SOURCES_TESTS
        tests/*.cpp
)

add_executable(experiment-ecs-tests)
target_link_libraries(experiment-ecs-tests
        Catch2::Catch2
)
target_sources(experiment-ecs-tests PUBLIC ${SOURCES_TESTS})
target_include_directories(experiment-ecs-tests PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)