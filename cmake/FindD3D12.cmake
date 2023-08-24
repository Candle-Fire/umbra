
SET(D3D12_BIN_DIR ${dx12agility_SOURCE_DIR}/build/native/bin/x64)

SET(D3D12_INCLUDE_DIRS ${dx12agility_SOURCE_DIR}/build/native/include)
SET(D3D12_BINARY_DIRS ${D3D12_BIN_DIR})


function(add_dx12 target)
    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${D3D12_BINARY_DIRS}/D3D12Core.dll ${CMAKE_CURRENT_BINARY_DIR}/D3D12/D3D12Core.dll
            COMMAND_EXPAND_LISTS
    )
    target_link_libraries(${target} PRIVATE d3d12.dll)
endfunction()