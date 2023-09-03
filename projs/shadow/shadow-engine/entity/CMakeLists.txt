FILE(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)

SET(TESTS
        ${CMAKE_CURRENT_LIST_DIR}/tests/managers.test.cpp
)

target_shadow_module(shadow-engine
        SOURCES ${SOURCES}
        INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/inc/
        TESTS ${TESTS}
)