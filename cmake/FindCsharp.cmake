

if (LINUX)

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

set(DOTNET_BASE_OPTIONS
        /nologo
        /p:BuildingViaCMake=true
)


function(add_csharp_target target)

    SET(DOTNET_BUILD_DIR ${BUILD_DIR}/MSBuild/)

    set(options)
    set(oneValueArgs PROJECT_FILE)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(PARSE_ARGV 1 ARGUMENTS "${options}" "${oneValueArgs}"
            "${multiValueArgs}")

    if ("${ARGUMENTS_PROJECT_FILE}" STREQUAL "")
        SET(CS_PROJECT_NAME "${CMAKE_CURRENT_LIST_DIR}/${target}")
    else ()
        SET(CS_PROJECT_NAME "${CMAKE_CURRENT_LIST_DIR}/${ARGUMENTS_PROJECT_FILE}")
    endif ()
    STRING(APPEND CS_PROJECT_NAME ".csproj")


    set(TARGET_OPTIONS
            "/p:IntermediateOutputPath=${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/Dotnet_Build.dir/obj/"
            "/p:MSBuildProjectExtensionsPath=${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/Dotnet_Build.dir/obj/"
            "/p:OutDir=${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/Dotnet_Build.dir/bin/"
            "/p:PublishDir=${CMAKE_CURRENT_BINARY_DIR}"
    )

    LIST(APPEND TARGET_OPTIONS ${DOTNET_BASE_OPTIONS})

    execute_process(
            COMMAND ${DOTNET_COMMAND_PATH} msbuild ${CS_PROJECT_NAME} ${TARGET_OPTIONS}
            /t:Restore
    )

    execute_process(
            COMMAND ${DOTNET_COMMAND_PATH} msbuild ${CS_PROJECT_NAME} ${TARGET_OPTIONS}
            /t:GetCMakeOutputAssembly
            OUTPUT_VARIABLE OUTPUT_ASSEMBLY
    )
    string(STRIP "${OUTPUT_ASSEMBLY}" OUTPUT_ASSEMBLY)

    execute_process(
            COMMAND ${DOTNET_COMMAND_PATH} msbuild ${CS_PROJECT_NAME} ${TARGET_OPTIONS}
            /t:GetCMakeOutputByproducts
            OUTPUT_VARIABLE OUTPUT_BYPRODUCTS
    )
    string(STRIP "${OUTPUT_BYPRODUCTS}" OUTPUT_BYPRODUCTS)

    message("Output is: ${OUTPUT_ASSEMBLY}")

    add_custom_command(
            OUTPUT ${OUTPUT_ASSEMBLY}
            DEPENDS ${ARGUMENTS_SOURCES}
            BYPRODUCTS ${OUTPUT_BYPRODUCTS}
            COMMAND ${DOTNET_COMMAND_PATH} build ${CS_PROJECT_NAME} ${TARGET_OPTIONS} /t:Build,Publish
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )

    add_custom_target(${target}
            DEPENDS ${OUTPUT_ASSEMBLY}
    )

    set_target_properties(${target} PROPERTIES TARGET_FILE $<PATH:REPLACE_EXTENSION,${OUTPUT_ASSEMBLY},>)

endfunction()