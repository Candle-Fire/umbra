
FILE(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)

SET(IncludeDirs
        ${CMAKE_CURRENT_LIST_DIR}/inc
)

# ##################################################
# ################# PLATFORMS ######################
# ##################################################
# Windows
if (WIN32)
    FILE(GLOB_RECURSE WIN_PLATFORM_SOURCES
            ${CMAKE_CURRENT_LIST_DIR}/windows/src/*.cpp
    )
    LIST(APPEND SOURCES ${WIN_PLATFORM_SOURCES})
    list(APPEND IncludeDirs ${CMAKE_CURRENT_LIST_DIR}/windows/inc)
endif ()
# Linux
if (UNIX AND NOT APPLE)
    FILE(GLOB_RECURSE LINUX_PLATFORM_SOURCES
            ${CMAKE_CURRENT_LIST_DIR}/linux/src/*.cpp
    )
    LIST(APPEND SOURCES ${LINUX_PLATFORM_SOURCES})
    list(APPEND IncludeDirs ${CMAKE_CURRENT_LIST_DIR}/linux/inc)
endif ()


FILE(GLOB_RECURSE INDEPENDENT_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)
LIST(APPEND SOURCES ${INDEPENDENT_SOURCES})

target_shadow_module(shadow-engine
        SOURCES ${SOURCES}
        INCLUDE_DIR ${IncludeDirs}
)