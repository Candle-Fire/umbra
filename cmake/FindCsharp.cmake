

if(LINUX)

    execute_process(
            COMMAND which dotnet
            OUTPUT_VARIABLE DOTNET_COMMAND_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
            COMMAND ${DOTNET_COMMAND_PATH} --version
            OUTPUT_VARIABLE DotNetVersion
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    message("Using dotnet at: ${DOTNET_COMMAND_PATH} with version: ${DotNetVersion}")
elseif (WINDOWS)


    set(MSBuildPath "C:/Program Files/dotnet/sdk/7.0.203/MSBuild.dll")
endif ()


function(add_csharp_target target)

    SET(DOTNET_BUILD_DIR ${BUILD_DIR}/MSBuild/)

    set(options)
    set(oneValueArgs PROJECT_FILE)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(PARSE_ARGV 1 ARGUMENTS "${options}" "${oneValueArgs}"
            "${multiValueArgs}")

    if ("${ARGUMENTS_PROJECT_FILE}" STREQUAL "")
        SET(CS_PROJECT_NAME ${target})
    else ()
        SET(CS_PROJECT_NAME ${ARGUMENTS_PROJECT_FILE})
    endif ()

    SET(OUTPUT "${DOTNET_BUILD_DIR}/bin/Debug/net7.0/arch-x64/${CS_PROJECT_NAME}")

    add_custom_target(${target}
            COMMAND ${DOTNET_COMMAND_PATH} build ${CS_PROJECT_NAME}.sln
                --property:BaseOutputPath=${DOTNET_BUILD_DIR}/bin/
                --property:BaseIntermediateOutputPath=${DOTNET_BUILD_DIR}/obj/
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${ARGUMENTS_SOURCES}
            BYPRODUCTS ${OUTPUT}
    )

endfunction()