#pragma once

/**
 * Implements base functionality for the engine (renderer, specifically) to integrate with the Shader subsystem.
 * This file (and all system-specific shader headers) are included by the shaders themselves as well as the engine.
 * This ensures that they "speak the same language".
 */

#ifdef __cplusplus
// Shaders don't run this code! convenient, right?

    using float4x4 = DirectX::XMFLOAT4X4;
    using float2 = DirectX::XMFLOAT2;
    using float3 = DirectX::XMFLOAT3;
    using float4 = DirectX::XMFLOAT4;
    using uint = uint32_t;
    using uint2 = DirectX::XMUINT2;
    using uint3 = DirectX::XMUINT3;
    using uint4 = DirectX::XMUINT4;
    using int2 = DirectX::XMINT2;
    using int3 = DirectX::XMINT3;
    using int4 = DirectX::XMINT4;

#define column_major
#define row_major

#define CB_GETSLOT(name) __CBUFFERBINDSLOT_##name##__
#define CBUFFER(name, slot) static const int CB_GETBINDSLOT(name) = slot; struct alignas(16) name
#define CONSTANTBUFFER(name, type, slot) // Use when needing to define a buffer in shaders, from module code
#define PUSHCONSTANT(name, type) // use Vulkan push constants if available, otherwise use DX constant buffers like normal. Bind slot is b999.

#else
// Shaders run this code now! We can have shaderside definitions of the above..

#define PASTE1(a, b) a##b
#define PASTE(a, b) PASTE1(a, b)
#define CBUFFER(name, slot) cbuffer name : register(PASTE(b, slot))
#define CONSTANTBUFFER(name, type, slot) ConstantBuffer< type > name : register(PASTE(b, slot))

#if defined(__spirv__)
#define PUSHCONSTANT(name, type) [[vk::push_constant]] type name;
#else
#define PUSHCONSTANT(name, type) ConstantBuffer<type> name : register(b999)
#endif

#endif

#define BUFFER_IMAGE 0
#define BUFFER_FONT 0

#define BUFFER_RENDERER_FRAME 0
#define BUFFER_RENDERER_CAMERA 1
#define BUFFER_RENDERER_LIGHTMASK 2
#define BUFFER_RENDERER_LIGHT 3
#define BUFFER_RENDERER_VOXEL 3
#define BUFFER_RENDERER_TRACED 2
#define BUFFER_RENDERER_MISC 3

#define BUFFER_EMITTEDPARTICLE 4
#define BUFFER_HAIRPARTICLE 4
#define BUFFER_FFT 3
#define BUFFER_OCEAN_CONSTANT 3
#define BUFFER_OCEAN_FRAME 4
#define BUFFER_OCEAN_RENDER 3
#define BUFFER_CLOUD 3
#define BUFFER_SORT 4
#define BUFFER_MSAO 4
#define BUFFER_FSR 4
