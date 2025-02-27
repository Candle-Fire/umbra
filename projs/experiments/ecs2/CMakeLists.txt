set(CMAKE_CXX_STANDARD 23)

add_executable(experiment-ecs2)
add_executable(experiment::ecs2 ALIAS experiment-ecs2)


FILE(GLOB_RECURSE SOURCES
        src/*.cpp
)

target_sources(experiment-ecs2 PUBLIC ${SOURCES})
target_include_directories(experiment-ecs2 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(experiment-ecs2
        PRIVATE fmt::fmt-header-only
)


FILE(GLOB_RECURSE SOURCES_TESTS
        src/id_system.cpp
        src/span_dynamic.cpp
        src/ecs.exp.cpp
        tests/*.cpp
)

add_executable(experiment-ecs2-tests)
target_link_libraries(experiment-ecs2-tests
        PRIVATE Catch2::Catch2WithMain
)
target_compile_options(experiment-ecs2-tests
    PUBLIC -fconcepts-diagnostics-depth=4
)
target_sources(experiment-ecs2-tests PUBLIC ${SOURCES_TESTS})
target_include_directories(experiment-ecs2-tests PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)