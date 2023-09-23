find_package(Csharp REQUIRED)


FILE(GLOB_RECURSE SOURCES src/*.cs)

add_csharp_target(shadow_header_tool
        SOURCES ${SOURCES}
        PROJECT_FILE shadow-header-tool
)

#set(SHADOW_HEADER_TOOL_EXECUTABLE ${PROJECT_SOURCE_DIR}/bin/Debug/MSBuild/net7.0/win-x64/shadow-header-tool.exe)


function(shadow_header_tool_gen target)
    set(options)
    set(oneValueArgs OUTPUT)
    set(multiValueArgs INCLUDE_DIRS)
    cmake_parse_arguments(PARSE_ARGV 1 ARGUMENTS "${options}" "${oneValueArgs}"
            "${multiValueArgs}")

    get_target_property(SOURCES ${target} SOURCES)
    list(REMOVE_ITEM SOURCES ARGUMENTS_OUTPUT)

    get_target_property(INCLUDES ${target} INCLUDE_DIRECTORIES)

    get_target_property(SHADOW_HEADER_TOOL_EXECUTABLE shadow_header_tool TARGET_FILE)
    #set(SHADOW_HEADER_TOOL_EXECUTABLE "/data/programming/_Projects/umbra/umbra/bin/Debug/cmake-build/MinGW/projs/shadow/shadow-header-tool/shadow-header-tool")


    #add_custom_command(
    #        COMMAND ${SHADOW_HEADER_TOOL_EXECUTABLE} generate_v2 -l minimal -p "${target}" -o "${ARGUMENTS_OUTPUT}"
    #        "-f:$<JOIN:${SOURCES},;-f:>"
    #        "-i:$<JOIN:${INCLUDES},;-i:>"
    #
    #        DEPENDS ${SOURCES} shadow_header_tool always_build
    #        OUTPUT ${ARGUMENTS_OUTPUT}
    #        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    #        COMMENT "Reflection information generator"
    #        COMMAND_EXPAND_LISTS
    #)

    add_custom_target("${target}_gen"
            COMMAND ${SHADOW_HEADER_TOOL_EXECUTABLE} generate_v2 -l minimal -p "${target}" -o "${ARGUMENTS_OUTPUT}"
            "-f:$<JOIN:${SOURCES},;-f:>"
            "-i:$<JOIN:${INCLUDES},;-i:>"
            DEPENDS ${SOURCES} shadow_header_tool always_build
            BYPRODUCTS ${ARGUMENTS_OUTPUT}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Reflection information generator"
            COMMAND_EXPAND_LISTS
    )

    target_sources(${target} PRIVATE ${ARGUMENTS_OUTPUT})

endfunction()

# Dummy output which is never actually produced. Anything that depends on
# this will always be rebuilt.
add_custom_target(always_build
        COMMAND cmake -E echo
)

