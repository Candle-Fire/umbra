
SET(TESTS
        ${CMAKE_CURRENT_LIST_DIR}/test.main.cpp

)
target_shadow_module(shadow-engine
        INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/inc/
        TESTS ${TESTS}
)