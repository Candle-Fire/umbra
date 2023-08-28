
function(target_shadow_module target)

    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES INCLUDE_DIR)
    cmake_parse_arguments(PARSE_ARGV 1 ARGUMENTS "${options}" "${oneValueArgs}"
            "${multiValueArgs}")

    target_sources(${target} PRIVATE ${ARGUMENTS_SOURCES})
    target_include_directories(${target}
            PUBLIC ${ARGUMENTS_INCLUDE_DIR}
    )

endfunction()