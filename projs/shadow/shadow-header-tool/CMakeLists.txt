
set(GENERATE_COMMAND P:/_Projects/umbra/umbra/bin/Debug/MSBuild/net7.0/win-x64/shadow-header-tool.exe)

function(shadow_header_tool_gen)
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
            COMMAND "${GENERATE_COMMAND}" "generate --project ${TARGET_NAME} -o ${SH_GENERATED}"
            DEPENDS ${DEPENDENCIES}
            OUTPUT ${SH_GENERATED}
            COMMENT "Reflection information generator"
    )

endfunction()

