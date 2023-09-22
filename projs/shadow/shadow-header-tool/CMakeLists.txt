find_package(Csharp REQUIRED)


FILE(GLOB_RECURSE SOURCES src/*.cs)

set(SHADOW_HEADER_TOOL_EXECUTABLE ${PROJECT_SOURCE_DIR}/bin/Debug/MSBuild/net7.0/win-x64/shadow-header-tool.exe)

add_csharp_target(shadow_header_tool
        SOURCES ${SOURCES}
        PROJECT_FILE shadow-header-tool
)

function(shadow_header_tool_gen target sources output)
    set(GENERATE_COMMAND ${SHADOW_HEADER_TOOL_EXECUTABLE})

    set(SH_GENERATED ${output})

    #execute_process(
    #        COMMAND "${GENERATE_COMMAND}" dependencies -nb -p "${target}"
    #        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    #        OUTPUT_VARIABLE DEPENDENCIES
    #        RESULT_VARIABLE RETURN_VALUE
    #
    #        COMMAND_ECHO STDOUT
    #)
    #if (NOT RETURN_VALUE EQUAL 0)
    #    message(FATAL_ERROR "Failed to get the dependencies")
    #endif ()

    add_custom_command(
            COMMAND "${GENERATE_COMMAND}" generate -nb -l minimal -p "${target}" -o "${SH_GENERATED}" --cmake-folder "${CMAKE_CACHEFILE_DIR}"
            DEPENDS ${sources} shadow_header_tool
            OUTPUT ${SH_GENERATED}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Reflection information generator"
    )

    target_sources(${target} PRIVATE ${SH_GENERATED})

endfunction()

