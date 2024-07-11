#pragma once
#include <string>
#include <vector>
#include <cstddef>
#include <limits>
#include <memory>
#include <cassert>
#include "shadow/traits/Bitmask.h"

/**
 * Contains enums and structs that are used by Render classes.
 * They are stored here for brevity.
 *
 * Even with the helper macros, this file is still 2414 lines long! Collapse those namespaces and open just the one you want.
*/

// An enum that contains powers of two only.
#define bitfield enum class
#define bit(x) (1 << x)

// A POD struct, that contains no state, only defines how other (stateful) structs should be created.
#define descriptor struct

#define graphicsItem(x) struct x : public GraphicsDeviceItem
#define resource(x) struct x : public GPUResource

// A simple wrapper for objects that hold a metadata object and expose it via the const function.
#define metaHolder(x) x meta; constexpr const x& getMeta() const { return meta; }

// Enums
namespace rx {
    namespace defs {
        enum class BlendModes : uint32_t {
            OPAQUE,
            ALPHA,
            PREMULTIPLIED, // color is normally sqrt
            ADDITIVE,
            MULTIPLY,
            SIZE
        };

        enum class Filter : uint32_t {
            NONE = 0,
            OPAQUE = 1,
            TRANSPARENT = 2,
            WATER = 4,
            NAVMESH = 8,
            OBJECTS = OPAQUE | TRANSPARENT | WATER | NAVMESH,

            COLLIDER = 16,
            ALL = OBJECTS | COLLIDER
        };

        enum class RenderPass : uint32_t {
            MAIN,
            PRE,
            ENVMAPPING,
            SHADOW,
            VOXEL,
            SIZE
        };

        enum class StencilMask : uint32_t {
            ENGINE = 0xF,
            USER = 0xF0,
            ALL = ENGINE | USER,
            SIZE
        };

        enum class StencilReference : uint32_t {
            EMPTY = 0,
            DEFAULT = 1,
            CUSTOMSHADER = 2,
            OUTLINE = 3,
            CUSTOMSHADER_OUTLINE = 4,
            LAST = 15
        };

        enum class BufferType : uint32_t {
            FRAME,
            ENTITY,
            SIZE
        };

        enum class TextureType : uint32_t {
            CLOUD_SHADOW,
            ATMO_TRANSMITTANCE,
            ATMO_SCATTER,
            ATMO_SKY,
            ATMO_SKYLUM,
            ATMO3_CAM,
            SHEEN,
            WIND3,
            SIZE
        };

        enum class ShaderType : uint32_t {
            V_OBJECT_DEBUG,
            V_OBJECT_COMMON,
            V_OBJECT_SIMPLE,
            V_OBJECT_PRE,
            V_OBJECT_PRE_ALPHA,
            V_OBJECT_COMMON_TESS,
            V_OBJECT_PRE_TESS,
            V_OBJECT_PRE_ALPHA_TESS,
            V_OBJECT_SIMPLE_TESS,

            V_SHADOW,
            V_SHADOW_ALPHA,
            V_SHADOW_TRANSPARENT,

            V_BILLBOARD,
            V_VERTEX,
            V_LIGHT_DIRECT,
            V_LIGHT_POINT,
            V_LIGHT_SPOT,
            V_LIGHT_SPOT_VISUAL,
            V_LIGHT_POINT_VISUAL,
            V_SKY,
            V_ENVMAP,
            V_ENVMAP_SKY,
            V_SPHERE,
            V_OCCLUDED,
            V_VOXELIZER,
            V_VOXEL,
            V_FFVISUAL_POINT,
            V_FFVISUAL_PLANE,
            V_LIGHTMAP,
            V_RAYTRACE,
            V_POSTPROCESS,
            V_FLARE,
            V_DDGI,

            /** Fragment */

            F_OBJECT_PERMUTE_BEGIN,
            F_OBJECT_PERMUTE_END,
            F_OBJECT_TRANSPARENT_PERMUTE_BEGIN,
            F_OBJECT_TRANSPARENT_PERMUTE_END,
            F_BILLBOARD,
            F_OBJECT_HOLOGRAM,
            F_OBJECT_DEBUG,
            F_OBJECT_SIMPLE,
            F_OBJECT_PRE,
            F_OBJECT_PRE_ALPHA,
            F_BILLBOARD_PRE,
            F_BILLBOARD_SIMPLE,

            F_VERTEXCOLOR,
            F_LIGHTVISUAL,
            F_LIGHT_DIRECT,
            F_LIGHT_POINT,
            F_LIGHT_SPOT,
            F_SKY_STATIC,
            F_SKY_DYNAMIC,
            F_SUN,
            F_ENVMAP,
            F_ENVMAP_SKY_STATIC,
            F_ENVMAP_SKY_DYNAMIC,
            F_CUBEMAP,
            F_CAPTUREBILLBOARDS,
            F_VOXELIZER,
            F_VOXEL,
            F_FFVISUAL,
            F_RT_DEBUGBV,
            F_DOWNSAMPLE,
            F_POST_UPSAMPLE,
            F_POST_OUTLINE,
            F_LENSFLARE,
            F_DDGI,
            F_POST_CLOUDS_UPSAMPLE,

            /** Geometry */

            G_SHADOW_EMULATE,
            G_SHADOW_ALPHA_EMULATE,
            G_SHADOW_TRANSPARENT_EMULATE,
            G_ENVMAP_EMULATE,
            G_ENVMAP_SKY_EMULATE,
            G_VOXELIZER,
            G_VOXEL,
            G_OBJECT_PRIMITIVE_EMULATE,
            G_OBJECT_PRIMITIVE_EMULATE_ALPHA,

            /** Hull */

            H_OBJECT,
            H_OBJECT_PRE,
            H_OBJECT_PRE_ALPHA,
            H_OBJECT_SIMPLE,

            /** Domain */

            D_OBJECT,
            D_OBJECT_PRE,
            D_OBJECT_PRE_ALPHA,
            D_OBJECT_SIMPLE,

            /** Compute */

            C_LUM1,
            C_LUM2,
            C_SRC,
            C_SRC_DEBUG,
            C_TILE_FRUSTUM,
            C_LIGHTCULL,
            C_LIGHTCULL_DEBUG,
            C_LIGHTCULL_ADV,
            C_LIGHTCULL_ADV_DEBUG,
            C_RESOLVE_MSAA,
            C_VXGI_OFFSET,
            C_VXGI_TEMPORAL,
            C_VXGI_SDF,
            C_VXGI_DIFFUSE,
            C_VXGI_SPECULAR,
            C_ATMO_TRANSMITTANCE,
            C_ATMO_SCATTER,
            C_ATMO_SKY,
            C_ATMO_SKY_LUM,
            C_ATMO_CAM,
            C_MIP_2UNORM4,
            C_MIP_2FLOAT4,
            C_MIP_3UNORM4,
            C_MIP_CUBE_UNORM4,
            C_MIP_CUBE_FLOAT4,
            C_MIP_CUBE_ARRAY_UNORM4,
            C_MIP_CUBE_ARRAY_FLOAT4,
            C_COMPRESS_1,
            C_COMPRESS_2,
            C_COMPRESS_3,
            C_COMPRESS_4,
            C_COMPRESS_5,
            C_COMPRESS_6H,
            C_COMPRESS_6H_CUBE,
            C_FILTERENV,
            C_COPY_2UNORM4,
            C_COPY_2FLOAT4,
            C_COPY_2UNORM4_EXPAND,
            C_COPY_2FLOAT4_EXPAND,
            C_SKINNING,
            C_RAYTRACE,
            C_PAINT_TEXTURE,
            C_POST_BLUR_GAUSS_F1,
            C_POST_BLUR_GAUSS_F3,
            C_POST_BLUR_GAUSS_F4,
            C_POST_BLUR_GAUSS_UNORM1,
            C_POST_BLUR_GAUSS_UNORM4,
            C_POST_BLUR_GAUSS_WIDE_F1,
            C_POST_BLUR_GAUSS_WIDE_F3,
            C_POST_BLUR_GAUSS_WIDE_F4,
            C_POST_BLUR_GAUSS_WIDE_UNORM1,
            C_POST_BLUR_GAUSS_WIDE_UNORM4,
            C_POST_BLUR_BILAT_F1,
            C_POST_BLUR_BILAT_F3,
            C_POST_BLUR_BILAT_F4,
            C_POST_BLUR_BILAT_UNORM1,
            C_POST_BLUR_BILAT_UNORM4,
            C_POST_BLUR_BILAT_WIDE_F1,
            C_POST_BLUR_BILAT_WIDE_F3,
            C_POST_BLUR_BILAT_WIDE_F4,
            C_POST_BLUR_BILAT_WIDE_UNORM1,
            C_POST_BLUR_BILAT_WIDE_UNORM4,
            C_POST_SSAO,
            C_POST_HBAO,
            C_POST_MSAO_PREPARE1,
            C_POST_MSAO_PREPARE2,
            C_POST_MSAO_INTERLEAVE,
            C_POST_MSAO,
            C_POST_MSAO_BLUR_UPSAMPLE,
            C_POST_MSAO_BLUR_UPSAMPLE_OUT,
            C_POST_MSAO_BLUR_UPSAMPLE_PRE,
            C_POST_MSAO_BLUR_UPSAMPLE_PRE_OUT,
            C_POST_RT_REFLECT,
            C_POST_RT_DIFFUSE,
            C_POST_RT_DIFFUSE_SPATIAL,
            C_POST_RT_DIFFUSE_TEMPORAL,
            C_POST_RT_DIFFUSE_BILAT,
            C_POST_SSR_TILEHORIZONTAL,
            C_POST_SSR_TILEVERTICAL,
            C_POST_SSR_DEPTH,
            C_POST_SSR_RAYTRACE,
            C_POST_SSR_RAYTRACE_SHORT,
            C_POST_SSR_RAYTRACE_CHEAP,
            C_POST_SSR_RESOLVE,
            C_POST_SSR_TEMPORAL,
            C_POST_SSR_BILAT,
            C_POST_LIGHTSHAFTS,
            C_POST_DOF_TILEHORIZONTAL,
            C_POST_DOF_TILEVERTICAL,
            C_POST_DOF_PRE,
            C_POST_DOF_PRE_SHORT,
            C_POST_DOF_MAIN,
            C_POST_DOF_MAIN_SHORT,
            C_POST_DOF_MAIN_CHEAP,
            C_POST_DOF_POST,
            C_POST_DOF_UPSAMPLE,
            C_POST_MOTION_TILEHORIZONTAL,
            C_POST_MOTION_TILEVERTICAL,
            C_POST_MOTION_NEIGHBORMAX,
            C_POST_MOTION,
            C_POST_MOTION_SHORT,
            C_POST_MOTION_CHEAP,
            C_POST_BLOOM,
            C_POST_AERIAL,
            C_POST_AERIAL_CAP,
            C_POST_AERIAL_CAP_MSAA,
            C_POST_CLOUDS_SHAPE,
            C_POST_CLOUDS_DETAIL,
            C_POST_CLOUDS_CURL,
            C_POST_CLOUDS_WEATHER,
            C_POST_CLOUDS_RENDER,
            C_POST_CLOUDS_RENDER_CAP,
            C_POST_CLOUDS_RENDER_CAP_MSAA,
            C_POST_CLOUDS_REPROJECT,
            C_POST_CLOUDS_SHADOW,
            C_POST_CLOUDS_SHADOW_FILTER,
            C_POST_FXAA,
            C_POST_TXAA,
            C_POST_SHARPEN,
            C_POST_TONEMAP,
            C_POST_UNDERWATER,
            C_POST_FSR_UPSCALE,
            C_POST_FSR_SHARPEN,
            C_POST_FSR2_REACTIVE,
            C_POST_FSR2_LUMINANCE,
            C_POST_FSR2_PREPARE_COLOR,
            C_POST_FSR2_RECONSTRUCT_DEPTH,
            C_POST_FSR2_DEPTH_CLIP,
            C_POST_FSR2_LOCK,
            C_POST_FSR2_ACCUMULATE,
            C_POST_FSR2_RCAS,
            C_POST_CHROMATIC_ABERRATION,
            C_POST_UPSAMPLE_FLOAT1,
            C_POST_UPSAMPLE_UNORM1,
            C_POST_UPSAMPLE_FLOAT4,
            C_POST_UPSAMPLE_UNORM4,
            C_POST_UPSAMPLE_UINT4,
            C_POST_DOWNSAMPLE,
            C_POST_NORMALS,
            C_POST_SCREENSPACE_SHADOW,
            C_POST_RT_SHADOW,
            C_POST_RT_SHADOW_DENOISE_TILE,
            C_POST_RT_SHADOW_DENOISE_FILTER,
            C_POST_RT_SHADOW_DENOISE_TEMPORAL,
            C_POST_RTAO,
            C_POST_RTAO_DENOISE_TILE,
            C_POST_RTAO_DENOISE_FILTER,
            C_POST_SURFEL_COVERAGE,
            C_POST_SURFEL_PREPARE,
            C_POST_SURFEL_RAYTRACE,
            C_POST_SURFEL_UPDATE,
            C_POST_SURFEL_OFFSETS,
            C_POST_SURFEL_BINNING,
            C_POST_SURFEL_INTEGRATE,
            C_POST_VISIBILITY_RESOLVE,
            C_POST_VISIBILITY_RESOLVE_MSAA,
            C_POST_VISIBILITY_SKY,
            C_POST_VISIBILITY_VELOCITY,
            C_POST_VISIBILITY_SURFACE_REDUCED_PERMUTE_BEGIN,
            C_POST_VISIBILITY_SURFACE_REDUCED_PERMUTE_END,
            C_POST_VISIBILITY_SURFACE_PERMUTE_BEGIN,
            C_POST_VISIBILITY_SURFACE_PERMUTE_END,
            C_POST_VISIBILITY_SHADE_PERMUTE_BEGIN,
            C_POST_VISIBILITY_SHADE_PERMUTE_END,
            C_DDGI_RAYTRACE,
            C_DDGI_UPDATE,
            C_DDGI_UPDATE_DEPTH,
            C_TERRAIN_COLOR,
            C_TERRAIN_NORMAL,
            C_TERRAIN_SURFACE,
            C_MESH_PREPARE,
            C_BILLBOARD_PREPARE,
            C_VIRTUAL_REQUESTS,
            C_VIRTUAL_ALLOCATE,
            C_VIRTUAL_UPDATE,
            C_WIND,
            C_YUV_RGB,

            RTTYPE_REFLECT,

            SIZE
        };

        enum class InputLayout : uint32_t {
            OBJECT_DEBUG,
            LIGHTMAP,
            VERTEX_COLOR,
            POSITION,
            SIZE
        };

        enum class RasterizerState : uint32_t {
            FRONT,
            BACK,
            BOTH,
            WIREFRAME,
            WIREFRAME_SMOOTHED,
            WIREFRAME_BOTH,
            WIREFRAME_BOTH_SMOOTHED,
            SHADOW,
            SHADOW_BOTH,
            OCCLUDED,
            VOXELIZE,
            SKY,
            SIZE
        };

        enum class DepthStencilState : uint32_t {
            DEFAULT,
            TRANSPARENT,
            SHADOW,
            DISABLED,
            READONLY,
            READ_EQUAL,
            ENVMAP,
            CAPTURE_BILLBOARD,
            WRITE_ONLY,
            HOLOGRAM,
            SIZE
        };

        enum class BindState : uint32_t {
            OPAQUE,
            TRANSPARENT,
            ADDITIVE,
            PREMULTIPLIED,
            COLOR_WRITE_DISABLED,
            MULTIPLY,
            TRANSPARENT_SHADOW,
            SIZE
        };

        enum class SamplerType : uint32_t {
            OBJECT_SHADER,
            LINEAR_CLAMP,
            LINEAR_WRAP,
            LINEAR_MIRROR,
            POINT_CLAMP,
            POINT_WRAP,
            POINT_MIRROR,
            ANISO_CLAMP,
            ANISO_WRAP,
            ANISO_MIRROR,
            COMPARE_DEPTH,

            SIZE
        };
    }
}

// Metadata
namespace rx {
    /**
     * The status of validation layers.
     * Also affects the logging of the renderer modules internally.
     */
    enum class Validation : uint32_t {
        DISABLED,  // No validation messages.
        CPU_ONLY,  // Command validation only
        CPU_GPU,   // Include GPU validation too
        VERBOSE,   // All errors, warnings and info. Not always useful!
    };

    /**
     * The "class" of the render device.
     * Determines the render capabilities, and sometimes informs us internally about how much memory we can afford to use regularly.
     */
    enum class RenderDeviceType : uint32_t {
        OTHER,       // An unknown type of device that doesn't fit into the below categories.
        INTEGRATED,  // A GPU built into the physical CPU unit. Usually shares memory.
        DISCRETE,    // A GPU that is separate from the CPU entirely, usually connected via PCIe.
        VIRTUAL,     // A GPU that is given a portion of the processing power and memory of a discrete GPU, usually for render farms, or VPS.
        SOFTWARE     // A GPU that doesn't exist - fallback to software (CPU) based rendering.
    };

    /**
     * Which type of device the application should prefer to take as the primary device.
     * Defaults to discrete.
     */
    enum class RenderDeviceTypePreference : uint32_t {
        DISCRETE,
        INTEGRATED
    };

    /**
     * The stage in the render pipeline that a shader should apply to.
     * Modern rendering pipelines have a lot of stages.
     * The ones you probably want are VS (Vertex Shader) and FS (Fragment Shader).
     * Listed in order as per the Khronos docs.
     */
    enum class ShaderStage : uint32_t {
        MS,  // Mesh Shader.
        AS,  // Amplification Shader.
        VS,  // Vertex Shader.
        HS,  // Hull Shader.
        DS,  // Domain Shader.
        GS,  // Geometry Shader.
        FS,  // Fragment Shader.
        CS,  // Compute Shader.
        LIB, // Shader library (package of other shaders)
        Size // Number of elements in the enum.
    };

    /**
     * The engine accepts shaders in a lot of formats. Generally, this will either be HLSL6 or SPIRV.
     */
    enum class ShaderFormat : uint32_t {
        NONE,   // Unused?
        HLSL5,  // DX11
        HLSL6,  // DX12
        SPIRV   // Vulkan
    };

    /**
     * DirectX HLSL shaders expand capabilities with a "model" system.
     * Supported model depends on the active driver.
     * Not relevant for SPIRV shaders.
     */
    enum class ShaderModel : uint32_t {
        SM50, // Shader Model 5.0
        SM60, // 6.0
        SM61, // 6.1
        SM62, // 6.2
        SM63, // 6.3
        SM64, // 6.4
        SM65, // 6.5
        SM66, // 6.6
        SM67  // 6.7
    };

    /**
     * When preparing a vertex buffer for upload, we need to tell the renderer what kind of format we're sending.
     * Usually, it's a list of triangles. However, there are other ways to send geometry, such as a strip where each triangle shares a face (which is 33% more efficient!).
     * Each render API has its own constants for this, so we expose our own here.
     */
    enum class Topology : uint32_t {
        UNDEFINED,
        TRIANGLELIST,  // point1, point2, point3. point4, point5, point6.
        TRIANGLESTRIP, // point1, point2, point3. (point2, point3), point4. (point3, point4), point5.
        POINTLIST,     // point1. point2. point3. point4. point5. point6.
        LINELIST,      // point1, point2. point3, point4. point5, point6.
        LINESTRIP,     // point1, point2. (point2), point3. (point3), point4.
        PATCHLIST,     // DX moment?
    };

    /**
     * When comparing pixel values in a buffer, there are all kinds of functions we can use in the comparison.
     * By default, less than or equal.
     */
    enum class ComparisonFunc : uint32_t {
        NEVER,          // Always false.
        LESS,           // <
        EQUAL,          // ==
        LESS_EQUAL,     // <=
        GREATER,        // >
        NOT_EQUAL,      // !=
        GREATER_EQUAL,  // >=
        ALWAYS,         // Always true.
        SIZE
    };

    /**
     * A simple way to mask out (disable) the depth buffer for writing.
     * Useful when rendering some object (a hologram?) that shouldn't occlude other objects, or otherwise affect depth calculations through those vertexes.
     */
    enum class DepthWriteMask : uint32_t {
        MASKED,   // Writing is Disabled
        UNMASKED, // Writing is Enabled
    };

    /**
     * When using a stencil buffer, you can specify what should happen "outside" the stencil (in the transparent pixels).
     * These allow you to simulate a lot of retro style visual effects.
     */
    enum class StencilOp : uint32_t {
        KEEP,                   // Do Nothing
        DISCARD,                // Set the pixel to the clear color.
        REPLACE,                // Set the pixel to a specified color.
        INCREASE_SATURATION,    // "brighten" the pixels
        DECREASE_SATURATION,    // "darken" the pixels
        INVERT,                 // Swap the color with its complement.
        INCR,                   // Unsure.
        DECR,                   // Unsure.
    };

    /**
     * When blending between multiple images, there are a lot of variables that can be referenced.
     * Documentation omitted because umbra is not expected to use blending.
     */
    enum class BlendData : uint32_t {
        CLEAR,
        ONE,
        SOURCE_COLOR,
        SOURCE_COLOR_INVERSE,
        SOURCE_ALPHA,
        SOURCE_ALPHA_INVERSE,
        DESTINATION_ALPHA,
        DESTINATION_ALPHA_INVERSE,
        DESTINATION_COLOR,
        DESTINATION_COLOR_INVERSE,
        SOURCE_ALPHA_SATURATION,
        BLENDING_FACTOR,
        BLENDING_FACTOR_ONE_MINUS, // 1 - BLENDING_FACTOR
        SOURCE1_COLOR,
        SOURCE1_COLOR_INVERSE,
        SOURCE1_ALPHA,
        SOURCE1_ALPHA_INVERSE
    };

    /**
     * When performing blending, there are a few operations that can be performed.
     * Documentation omitted because umbra is not expected to use blending.
     */
    enum class BlendOp : uint32_t {
        ADD,
        SUBTRACT,
        SUBTRACT_INVERSE,
        MINIMAL,
        MAXIMAL
    };

    /**
     * There are a couple ways we can interpret sets of vertices that are submitted.
     * Especially relevant in TRIANGLELIST and TRIANGLESTRIP modes, this will specify whether we want to fill in the faces, or just treat them as points connected by lines.
     */
    enum class DrawMode : uint32_t {
        WIREFRAME,  // Only lines connecting vertices.
        SOLID,      // Filled faces.
    };

    /**
     * When considering faces for rendering, there are a few options:
     * We can choose to remove some of the faces before they get submitted to the GPU.
     * This is a very simple and very easy optimization.
     */
    enum class CullingMode : uint32_t {
        NONE,   // Render all faces. The GPU will discard faces naturally via depth sorting.
        BACK,   // Backfaces (normal vectors pointing away from the view plane) are discarded before render begins.
        FRONT,  // Frontfaces (normal vectors pointing towards the view plane) are discarded before render begins. Generates a freaky effect.
    };

    /**
     * When submitting data to the GPU related to objects, we need to tell it whether this data is intended for a vertex (such as uniform buffers), or for the whole instance (such as position data).
     */
    enum class InputClassification : uint32_t {
        VERTEX_DATA,
        INSTANCE_DATA
    };

    /**
     * When an image is used on the GPU, there are a couple different "ways" it can be used.
     */
    enum class BufferUsage : uint32_t {
        DEVICE,     // Image only exists in GPU native memory. CPU cannot access.
        STAGING,    // Image exists on both CPU and GPU. The CPU can only write, the GPU can only read. Intended to copy to a DEVICE image.
        READBACK,   // Image exists on both CPU and GPU. The CPU can only read, the GPU can only write. Intended for compute shaders to return a result back to the CPU for later usage.
    };

    /**
     * When reading an image, we can sometimes index it out of bounds.
     * This tells the GPU how we should deal with oversampling.
     */
    enum class ImageTiling : uint32_t {
        WRAP,           // pixel % size - wrap back around to the start of the image, effectively tiling it to infinity.
        MIRROR,         // size - (pixel % size) - invert every even number of "iterations" out.
        CLAMP,          // Prevent access > size.
        BORDER,         // Repeat the edge pixel when indexed out of bounds.
        MIRROR_ONCE,    // Mirror the image side to size in x and y, once. Accesses > size*2 are prevented.
        SIZE
    };

    /**
     * When accessing an image, there are a few different kinds of filter kernels we can apply over top.
     * When i say a few, there's a lot.
     * Documentation excluded because... i mean, look at it.
     * You probably want Anisotropic.
     */
    enum class Filtering : uint32_t {
        MIN_MAG_MIP_POINT,
        MIN_MAG_POINT_MIP_LINEAR,
        MIN_POINT_MAG_LINEAR_MIP_POINT,
        MIN_POINT_MAG_MIP_LINEAR,
        MIN_LINEAR_MAG_MIP_POINT,
        MIN_LINEAR_MAG_POINT_MIP_LINEAR,
        MIN_MAG_LINEAR_MIP_POINT,
        MIN_MAG_MIP_LINEAR,
        ANISOTROPIC,

        COMPARISON_MIN_MAG_MIP_POINT,
        COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
        COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
        COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
        COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
        COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
        COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
        COMPARISON_MIN_MAG_MIP_LINEAR,
        COMPARISON_ANISOTROPIC,

        MINIMUM_MIN_MAG_MIP_POINT,
        MINIMUM_MIN_MAG_POINT_MIP_LINEAR,
        MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
        MINIMUM_MIN_POINT_MAG_MIP_LINEAR,
        MINIMUM_MIN_LINEAR_MAG_MIP_POINT,
        MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
        MINIMUM_MIN_MAG_LINEAR_MIP_POINT,
        MINIMUM_MIN_MAG_MIP_LINEAR,
        MINIMUM_ANISOTROPIC,

        MAXIMUM_MIN_MAG_MIP_POINT,
        MAXIMUM_MIN_MAG_POINT_MIP_LINEAR,
        MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
        MAXIMUM_MIN_POINT_MAG_MIP_LINEAR,
        MAXIMUM_MIN_LINEAR_MAG_MIP_POINT,
        MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
        MAXIMUM_MIN_MAG_LINEAR_MIP_POINT,
        MAXIMUM_MIN_MAG_MIP_LINEAR,
        MAXIMUM_ANISOTROPIC,
    };

    /**
     * When sampling the border of an image (with a filter kernel especially), we can set what it will read.
     */
    enum class SamplerBorderColor : uint32_t {
        TRANSPARENT_BLACK,
        OPAQUE_BLACK,
        OPAQUE_WHITE
    };

    /**
     * Images can come in a large amount of pixel layouts.
     * Usually, some variant of R G B A pixels with varying bit depth, represented as a number after the component.
     * They can either be represented as a float, unsigned int, signed int, or unsigned normalized value.
     */
    enum class ImageFormat : uint32_t {
        UNKNOWN,

        R32G32B32A32_FLOAT,
        R32G32B32A32_UINT,
        R32G32B32A32_SINT,

        R32G32B32_FLOAT,
        R32G32B32_UINT,
        R32G32B32_SINT,

        R16G16B16A16_FLOAT,
        R16G16B16A16_UNORM,
        R16G16B16A16_UINT,
        R16G16B16A16_SNORM,
        R16G16B16A16_SINT,

        R32G32_FLOAT,
        R32G32_UINT,
        R32G32_SINT,
        D32_FLOAT_S8X24_UINT,

        R10G10B10A2_UNORM,
        R10G10B10A2_UINT,
        R11G11B10_FLOAT,

        R8G8B8A8_UNORM,
        R8G8B8A8_UNORM_SRGB,
        R8G8B8A8_UINT,
        R8G8B8A8_SNORM,
        R8G8B8A8_SINT,
        // BGR
        B8G8R8A8_UNORM,
        B8G8R8A8_UNORM_SRGB,

        R16G16_FLOAT,
        R16G16_UNORM,
        R16G16_UINT,
        R16G16_SNORM,
        R16G16_SINT,

        D32_FLOAT,
        R32_FLOAT,
        R32_UINT,
        R32_SINT,
        D24_UNORM_S8_UINT,
        R9G9B9E5_SHARED,

        R8G8_UNORM,
        R8G8_UINT,
        R8G8_SNORM,
        R8G8_SINT,

        R16_FLOAT,
        D16_UNORM,
        R16_UNORM,
        R16_UINT,
        R16_SNORM,
        R16_SINT,

        R8_UNORM,
        R8_UINT,
        R8_SNORM,
        R8_SINT,

        // Not valid render target formats

        BC1_UNORM,
        BC1_UNORM_SRGB,
        BC2_UNORM,
        BC2_UNORM_SRGB,
        BC3_UNORM,
        BC3_UNORM_SRGB,
        BC4_UNORM,
        BC4_SNORM,
        BC5_UNORM,
        BC5_SNORM,
        BC6H_UF16,
        BC6H_SF16,
        BC7_UNORM,
        BC7_UNORM_SRGB,

        NV12, // NVENC?
    };

    /**
     * When querying the GPU, we can ask for some specific information.
     */
    enum class GPUQueryType : uint32_t {
        ELAPSED_TIME,       // How long the current execution has been going
        OCCLUDED_PIXELS,    // How many pixels passed depth testing (useful for % efficiency in render submission
        OCCLUDED_BINARY     // Whether any pixels passed the depth test.
    };

    /**
     * For efficiency, we can pack the index buffer with uint16_t instead of uint32_t if there aren't enough indices.
     */
    enum class IndexBufferFormat : uint32_t {
        UINT16,
        UINT32
    };

    /**
     * Samplers can view into different types of images. The value changes how they are accessed and "barriered" by the GPU.
     */
    enum class ImageViewType : uint32_t {
        SHADER_RESOURCE,    // Texture Sample
        UNORDERED_ACCESS,   // Data Sample
        RENDER_TARGET,      // Texture Output
        DEPTH_STENCIL       // Depth & Stencil Buffer
    };

    /**
     * When executing the fragment shader, we can batch multiple pixels in a line / square, to make it faster.
     */
    enum class ShadingRate : uint32_t {
        RATE_11, // 1x1, default.
        RATE_12, // 1x2
        RATE_21, // 2x1
        RATE_22, // 2x2
        RATE_24, // 2x4
        RATE_42, // 4x2
        RATE_44, // 4x4

        RATE_INVALID
    };

    /**
     * Certain types of operation can use a Predication, which is a weird way of saying "condition".
     */
    enum class PredicationOp : uint32_t {
        EQUAL_TO_ZERO,
        NOT_EQUAL_TO_ZERO
    };

    /**
     * An "aspect" is a usage, or a purpose.
     */
    enum class ImageAspect : uint32_t {
        COLOR,
        DEPTH,
        STENCIL,
        LUMINANCE,  // Black and white
        CHROMINANCE // Color only, no brightness
    };

    /**
     * Videos can either contain intra-frame data, or predict the data that should be between them.
     */
    enum class VideoFrameType : uint32_t {
        INTRA,
        PREDICTIVE
    };

    /**
     * Video data uses special encoding to make sure the filesize doesn't blow way out of proportion.
     */
    enum class VideoEncoding : uint32_t {
        H264 // We only support AVC H.264
    };

    /**
     * "Swizzle" is a way of swapping the order of color components.
     * This specifies the components that can partake in a swizzle.
     */
    enum class ComponentSwizzle : uint32_t {
        R,
        G,
        B,
        A,
        ZERO,
        ONE
    };

    /**
     * When writing colors, we can enable or disable certain colors by setting these bits.
     */
    bitfield ColorWrite {
        DISABLE = 0,
        ENABLE_RED = bit(0),
        ENABLE_GREEN = bit(1),
        ENABLE_BLUE = bit(2),
        ENABLE_ALPHA = bit(3),
        ENABLE_ALL = bit(4)
    };

    /**
     * When binding data to the GPU, we need to specify what kind of data it contains.
     */
    bitfield BindFlag {
        NONE = 0,
        VERTEX_BUFFER = bit(0),
        INDEX_BUFFER = bit(1),
        CONSTANT_BUFFER = bit(2),
        SHADER_RESOURCE = bit(3),
        RENDER_TARGET = bit(4),
        DEPTH_STENCIL = bit(5),
        UNORDERED_ACCESS = bit(6),
        SHADING_RATE = bit(7)
    };

    /**
     * When defining resources, we can specify all kinds of things about how we expect it to be used.
     * These inform the APIs and GPU about how it can optimize the resources, and when we can expect it to be readable.
     */
    bitfield ResourceFlags {
        NONE = 0,
        TEXTURE_CUBEMAP = bit(0),
        INDIRECT_ARGS = bit(1),  // DrawIndirect and DispatchIndirect require arguments sent to the shader
        BUFFER_RAW = bit(2),
        BUFFER_STRUCTURED = bit(3),
        RAY_TRACING = bit(4),
        PREDICATION = bit(5),
        TRANSIENT_ATTACHMENT = bit(6),  // Used only in the next render pass. Do not send to CPU memory.
        SPARSE = bit(7),                // No backing memory.
        SPARSE_BUFFER = bit(8),
        SPARSE_NON_RENDER = bit(9),
        SPARSE_RENDER = bit(10),
        SPARSE_POOL = SPARSE_BUFFER | SPARSE_NON_RENDER | SPARSE_RENDER,
        CAST_TYPED = bit(11),
        CAST_FORMATTED = bit(12),
        VIDEO_DECODE = bit(13),
        NO_DEFAULT_DESCRIPTORS = bit(14),
        SAMPLE_COMPRESSED_TEXTURE = bit(15)
    };

    /**
     * Certain graphics drivers expose more functionality than others.
     * Thus, there is a baseline functionality level that we can expect, as well as a couple nice extras.
     * This enumerates all of those nice extras that I expect umbra to use or need in the future.
     */
    bitfield GraphicsDeviceCapability {
        NONE = 0,
        TESSELLATION = bit(0), // Dynamic vertex bumping in the positive or negative direction, usually for on-GPU terrain gen.
        CONSERVATIVE_RASTERIZATION = bit(1),    // Helps minimise the render load a bit more.
        RASTERIZER_ORDERED_VIEWS = bit(2),      // Shaders should expect resources in the order they're used4
        UAV_LOAD_FORMAT_COMMON = bit(3),        // Unordered Access Views (samplers) should expect resources in common formats (RGBA16, RGBA8)
        UAV_LOAD_FORMAT_R11G11B10_FLOAT = bit(4), // HDR
        RT_VIEWPORT_WITHOUT_GEOMETRY_SHADER = bit(5),
        VARIABLE_RATE_SHADING = bit(6),
        VARIABLE_RATE_SHADING_TIER2 = bit(7),
        SUPPORTS_MESH_SHADER = bit(8),          // The first shader stage... if the driver supports it! Otherwise, usually Vertex Shader.
        RAY_TRACING = bit(9),                   // Hardware accelerated raytracing, specifically. We can still do it with a compute shader if this isn't supported. In fact, we will! Just to show them who's boss. It's me. I'm the boss around here.
        PREDICATION = bit(10),                  // Documentation is sparse. I assume this discards the render job entirely if there's no pixels.
        SAMPLER_MINMAX = bit(11),
        DEPTH_BOUNDS_TEST = bit(12),
        SPARSE_BUFFER = bit(13),                // Useful for storing very temporary data on GPU. It should assign you a little bucket of memory when the binary loads.
        SPARSE_TEXTURE2D = bit(14),             // Useful for storing a 2D buffer temporarily in the shader.
        SPARSE_TEXTURE3D = bit(15),             // Useful for storing a 3D buffer temporarily in the shader.
        SPARSE_NULL_MAPPING = bit(16),          // Map all uninitialized sparse buffers to null.
        GENERIC_SPARSe = bit(17),               // Support render and non-render sparse buffers.
        DEPTH_RESOLVE_MIN_MAX = bit(18),
        STENCIL_RESOLVE_MIN_MAX = bit(19),
        CACHE_COHERENT_UMA = bit(20),           // Some DirectX thing. Seems useful.
        VIDEO_DECODE_H264 = bit(21),            // Being able to decode video would be useful for our video decoder.
        SUPPORTS_R9G9B9E5_SHARED = bit(22),     // Supports this format.
    };

    /**
     * Resources can be in a lot of states.
     * Generally, we want them to be in another state.
     * This allows for an easy reference for barriering a resource.
     */
    bitfield ResourceState {
        UNDEFINED = 0,              // Discard contents
        SHADER_RESOURCE = bit(0),   // Probably a sampler. Read Only.
        SHADER_RESOURCE_COMPUTE = bit(1), // Probably a data buffer. Only for compute shaders.
        UNORDERED_ACCESS = bit(2),  // Probably an output buffer. Read+Write.
        COPY_SOURCE = bit(3),       // Intended as the source for copying - usually a staging buffer.
        COPY_DESTINATION = bit(4),  // Intended as the destination for copying - usually an image or data buffer.

        // Texture specific
        RENDER_TARGET = bit(5),     // Render output. Read+Write
        DEPTH_STENCIL = bit(6),
        DEPTH_STENCIL_RO = bit(7),      // Same as above, but read-only.
        SHADING_RATE_SOURCE = bit(8),   // Control shading rate per tile (allows optimizing larger areas where detail isn't more important).

        // Data buffer specific
        VERTEX_BUFFER = bit(9),
        INDEX_BUFFER = bit(10),
        CONSTANT_BUFFER = bit(11),
        INDIRECT_ARGUMENT = bit(12),    // DrawIndirect and DispatchIndirect require arguments sent to the shader
        RT_ACCELERATION = bit(13),
        PREDICATION = bit(14),

        // Video specific
        VIDEO_DECODE_SOURCE = bit(15),
        VIDEO_DECODE_DESTINATION = bit(16)
    };

    /**
     * When picking an image format, we need to decide which color space it will be in.
     * Color spaces are simple: There's SDR, and HDR.
     * If you don't know, it's SDR.
     */
    enum class ColorSpace {
        SRGB,
        HDR10,
        HDR16
    };

    /**
     * When setting up a render pass, we can configure what kind of behaviour it'll have.
     */
    enum class RenderPassFlags {
        NONE = 0, // Nothing special.
        ALLOW_UAV_WRITE, // Write into Unordered Access Views. Usually reserved for Compute.
        SUSPENDING, // Pause to wait for other stuff if necessary.
        RESUMING, // Kick other render passes that are suspended on this.
    };

    /**
     * A viewport defines the area on-screen that holds the current render target.
     * Usually, the whole game window.
     * Sometimes, a texture in-game for a camera to render to.
     * Sometimes, a smaller window for the game view in the editor.
     */
    descriptor Viewport {
        float topLeftX = 0;
        float topLeftY = 0;
        float width = 0;
        float height = 0;
        float minDepth = 0;
        float maxDepth = 1;
    };

    /**
     * The layout of shader inputs
     */
    descriptor InputLayout {
        static const uint32_t ALIGNED_ELEMENT = ~0u;

        struct Element {
            std::string name;
            uint32_t index = 0;
            ImageFormat format = ImageFormat::UNKNOWN;
            uint32_t slot = 0;
            uint32_t alignedOffset = ALIGNED_ELEMENT;
            InputClassification slotClass = InputClassification::VERTEX_DATA;
        };

        std::vector<Element> elements;
    };

    /**
     * When a new frame is starting, we can choose to keep the last frame's contents, or clear them and render a fresh version.
     * If we choose to clear them, we need to provide the value of which to replace the new pixels with.
     * For purely indoor games, this is typically removed, to optimize the render process.
     * Fun fact: this is what causes the weird visual stacking glitch in Source games out-of-bounds.
     */
    union ClearValue {
        float color[4];       // When clearing a color buffer, we use this.
        struct DepthStencil { // When clearing a depth-stencil buffer, we use this.
            float depth;
            uint32_t stencil;
        } depthStencil;
    };

    /**
     * If we want to change a format from RGBA to BGR, this is where we define that kind of operation.
     */
    descriptor Swizzle {
        ComponentSwizzle r = ComponentSwizzle::R;
        ComponentSwizzle g = ComponentSwizzle::G;
        ComponentSwizzle b = ComponentSwizzle::B;
        ComponentSwizzle a = ComponentSwizzle::A;
    };

    /**
     * When creating a texture, there's a lotta stuff we need to know about it beforehand.
     */
    descriptor TextureMeta {
        enum class Type {
            TEXTURE_1D,
            TEXTURE_2D,
            TEXTURE_3D
        } type = Type::TEXTURE_2D;

        uint32_t width = 1;  // Width of 0 is invalid.
        uint32_t height = 1; // Height of 0 is invalid.
        uint32_t depth = 1;  // Depth of 0 for 3D images is invalid.
        uint32_t arraySize = 1; // By default, only one image is needed. For cubemaps, we'd select 6.
        uint32_t mipLevels = 1; // Set mipping if required.
        ImageFormat format = ImageFormat::UNKNOWN;
        uint32_t sampleCount = 1; // For supersample aliasing, we can select multiple samples.
        BufferUsage usage = BufferUsage::DEVICE; // We want to create the image on the device by default, with a staging buffer for upload.
        BindFlag bindFlag = BindFlag::NONE;
        ResourceFlags resourceFlags = ResourceFlags::NONE;
        ClearValue clear = {};
        ResourceState layout = ResourceState::SHADER_RESOURCE; // By default, a texture is only used for sampling by a shader.
        Swizzle swizzle;
    };

    /**
     * When creating a sampler for a texture, there's a bunch of things we need to know, too.
     */
    descriptor SamplerMeta {
        Filtering filter = Filtering::MIN_MAG_MIP_POINT; // Yeah, i don't know either.
        ImageTiling uTiling = ImageTiling::CLAMP;
        ImageTiling vTiling = ImageTiling::CLAMP;
        ImageTiling wTiling = ImageTiling::CLAMP;
        uint32_t maxAnisotropy = 0;
        ComparisonFunc comparison = ComparisonFunc::NEVER;
        SamplerBorderColor borderColor = SamplerBorderColor::TRANSPARENT_BLACK;
        float minLOD = 0;
        float maxLOD = std::numeric_limits<float>::max();
        float mipLODBias = 0;
    };

    /**
     * We can configure how the renderer works when rasterizing geometry.
     * This does not apply to ray tracing, hence the "Rasterizer" prefix.
     */
    descriptor RasterizerState {
        DrawMode fillMode = DrawMode::SOLID;
        CullingMode culling = CullingMode::NONE;
        bool reverseWindingOrder = false; // Normal winding order is clockwise, looking "against" the normal.
        int32_t depthBias = 0;
        float depthBiasClamp = 0;
        float slopeDepthBias = 0;
        bool depthClipping = false;
        bool multisample = false;
        bool antialiasing = false;
        bool conservativeRasterization = false;
        uint32_t forcedSampleCount = 0;
    };

    /**
     * Just like regular images, we can configure the state of the depth stencil buffers.
     */
    descriptor DepthStencilState {
        bool depth = false;
        DepthWriteMask mask = DepthWriteMask::MASKED;
        ComparisonFunc comparison = ComparisonFunc::NEVER;
        bool stencil = false;
        uint8_t stencil_read = false;
        uint8_t stencil_write = false;

        struct DepthStencilOp {
            StencilOp fail = StencilOp::DISCARD;
            StencilOp depthFail = StencilOp::DISCARD;
            StencilOp pass = StencilOp::KEEP;
            ComparisonFunc stencil = ComparisonFunc::NEVER;
        };

        DepthStencilOp front;
        DepthStencilOp back;

        bool depthBoundsTest = false;
    };

    /**
     * Provided for brevity.
     * I don't expect umbra to use blending.
     */
    struct BlendState {
        bool alpha = false;
        bool independent = false;

        struct RenderTargetBlend {
            bool enable = false;
            BlendData source = BlendData::SOURCE_ALPHA;
            BlendData dest = BlendData::DESTINATION_ALPHA;
            BlendOp op = BlendOp::ADD;
            BlendData sourceAlpha = BlendData::ONE;
            BlendData destAlpha = BlendData::ONE;
            BlendOp opAlpha = BlendOp::ADD;
            ColorWrite writeMask = ColorWrite::ENABLE_ALL;
        };

        RenderTargetBlend targets[8];
    };
}

// Data
namespace rx {
    class Shader;
    class GPUResource;
    class GPUBuffer;
    class Texture;

    /**
     * When allocating a buffer for GPU consumption, there's a couple different things we can tell the render API to make it easier to optimize.
     */
    descriptor GPUBufferMeta {
        size_t size = 0;
        BufferUsage usage = BufferUsage::DEVICE;
        BindFlag binding = BindFlag::NONE;
        ResourceFlags flags = ResourceFlags::NONE;
        uint32_t stride = 0;
        ImageFormat format = ImageFormat::UNKNOWN;
        size_t alignment = 0;
    };

    /**
     * When submitting GPU Queries, we can submit a heap of them at once to be executed at intervals.
     */
    descriptor GPUQueryHeapMeta {
        GPUQueryType type = GPUQueryType::ELAPSED_TIME;
        uint32_t count = 0;
    };

    /**
     * When creating or updating a pipeline state object (PSO), we have granularity to each individual component we update.
     */
    descriptor PipelineStateMeta {
        const Shader* vertex = nullptr;
        const Shader* fragment = nullptr;
        const Shader* hull = nullptr;
        const Shader* domain = nullptr;
        const Shader* geometry = nullptr;
        const Shader* mesh = nullptr;
        const Shader* amplification = nullptr;
        const BlendState* blend = nullptr;
        const RasterizerState* rasterizer = nullptr;
        const DepthStencilState* depthStencil = nullptr;
        const InputLayout* layout = nullptr;
        Topology topology = Topology::TRIANGLELIST;
        uint32_t patchPoints = 3;
        uint32_t sampleMash = 0xFFFFFFFF;
    };

    /**
     * Barriers are a way of converting a resource from one format to another,
     * or for making sure that they're accessible a certain way at a certain time.
     *
     * The GPU uses this information to optimize where in memory it places things for rendering.
     * Proper usage of barriers can bring an astonishing performance boost.
     */
    descriptor BarrierType {
        enum Type {
            MEMORY,
            IMAGE,
            BUFFER
        } type = MEMORY;

        struct Memory {
            const GPUResource* resource;
        };

        struct Image {
            const Texture* texture;
            ResourceState layoutBefore;
            ResourceState layoutAfter;
            int mip;
            int slice;
            const ImageAspect* aspect;
        };

        struct Buffer {
            const GPUBuffer* buffer;
            ResourceState layoutBefore;
            ResourceState layoutAfter;
        };

        // The relevant entry is determined by the Type.
        union {
            Memory memory;
            Image image;
            Buffer buffer;
        };

        static Barrier Memory(const GPUResource* resource = nullptr) {
            Barrier barrier;
            barrier.type = Type::MEMORY;
            barrier.memory.resource = resource;
            return barrier;
        }

        static Barrier Image(const Texture* texture, ResourceState before, ResourceState after, int mip = -1, int slice = -1, const ImageAspect* aspect = nullptr) {
            Barrier barrier {
                .type = Type::IMAGE,
                .image = {
                    .texture = texture,
                    .layoutBefore = before,
                    .layoutAfter = after,
                    .mip = mip,
                    .slice = slice,
                    .aspect = aspect
                }
            };

            return barrier;
        }

        static Barrier Buffer(const GPUBuffer* buffer, ResourceState before, ResourceState after) {
            Barrier barrier {
                .type = Type::BUFFER,
                .buffer = {
                        .buffer = buffer,
                        .layoutBefore = before,
                        .layoutAfter = after
                }
            };

            return barrier;
        }
    };

    /**
     * A swapchain basically controls double/triple buffering, by automatically swapping out the image we render to after every frame is presented.
     */
    descriptor SwapchainMeta {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t buffers = 2;
        ImageFormat format = ImageFormat::R10G10B10A2_UNORM; // Try HDR?
        bool fullscreen = false;
        bool vsync = true;
        float clear[4] = { 0, 0, 0, 1 };
        bool hdr = true;
    };

    /**
     * A subresource is essentially a smaller resource embedded in a larger.
     * Think mipmaps, where the resource is the atlas of all mippings, and the subresource is every individual image.
     */
    descriptor SubresourceMeta {
        const void* data = nullptr;
        uint32_t rowPitch = 0;
        uint32_t slicePitch = 0;
    };

    // A rectangle.
    descriptor Rect {
        int32_t left = 0;
        int32_t top = 0;
        int32_t right = 0;
        int32_t bottom = 0;
    };

    // A cube
    descriptor Box {
        uint32_t left = 0;
        uint32_t top = 0;
        uint32_t front = 0;
        uint32_t right = 0;
        uint32_t bottom = 0;
        uint32_t back = 0;
    };

    // TODO
    descriptor SparseTextureMeta {
        uint32_t tileWidth = 0;
        uint32_t tileHeight = 0;
        uint32_t tileDepth = 0;
        uint32_t totalTiles = 0;
        uint32_t packedMipStart = 0;
        uint32_t packedMipCount = 0;
        uint32_t packedMipTileOffset = 0;
        uint32_t packedMipTileCount = 0;
    };

    /**
     * When encoding, decoding or playing a video, we need to know some things about it.
     * These specify how much memory it's going to use for decode and playback, as well as whether we need to postprocess (scale) it to play it properly.
     */
    descriptor VideoMeta {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t bitRate = 0;
        ImageFormat format = ImageFormat::NV12;
        VideoEncoding encoding = VideoEncoding::H264;
        const void* pictureParameters = nullptr;
        size_t pictureParameterCount = 0;
        const void* sequenceParameters = nullptr;
        size_t sequenceParameterCount = 0;
        uint32_t decodeBufferSlots = 0;
    };
}

// Resource
namespace rx {

    /**
     * Some kind of resource that lives "on" the graphics device, or an object handled by the device API.
     * Lots of stuff here that exists to make Vulkan happy.
     */
    descriptor GraphicsDeviceItem {
        std::shared_ptr<void> internal;

        inline bool isValid() const { return internal != nullptr; }

        virtual ~GraphicsDeviceItem() = default;
    };

    /**
     * A sampler allows a shader to read an image, with optional postprocessing effects, supersampling, blurring, whatever.
     */
    graphicsItem(Sampler) {
        SamplerMeta meta;

        const SamplerMeta &getMeta() const { return meta; }
    };

    /**
     * A shader is a binary blob that lives on GPU and must be bound with a pipeline to be used.
     */
    graphicsItem(Shader) {
        ShaderStage stage = ShaderStage::Size;
    };

    /**
     * Other kinds of resources - such as buffers, textures, and raytracing acceleration structures (Bounding Volume Hierarchies) - are treated more or less the same.
     */
    graphicsItem(GPUResource) {
        enum Type {
            BUFFER,
            TEXTURE,
            RAYTRACING,
            UNKNOWN
        } type = Type::UNKNOWN;

        constexpr bool isTexture() const { return type == Type::TEXTURE; }

        constexpr bool isBuffer() const { return type == Type::BUFFER; }

        constexpr bool isRT() const { return type == Type::RAYTRACING; }

        // Only valid for STAGING or READBACK resources.
        void *mapped = nullptr;
        size_t mappedSize = 0;

        size_t sparsePageSize = 0;
    };

    // A buffer just holds data - vertex buffers, index buffers, constant buffers, uniform buffers.
    resource(GPUBuffer) {
        metaHolder(GPUBufferMeta)
    };

    /**
     * A texture holds pixel data that can be mapped onto an object according to uv values in the pixel buffer.
     * Textures can hold subresources (such as textures with mipping - the mipped textures are subresources within the primary.)
     */
    resource(Texture) {
        TextureMeta meta;

        const SubresourceMeta *mappedResources = nullptr;
        size_t mappedResourceCount = 0;

        const SparseTextureMeta *sparse;

        constexpr const TextureMeta &getMeta() const { return meta; }
    };

    /**
     * Video Decoders are objects that live on the GPU too.
     */
    graphicsItem(VideoDecoder) {
        metaHolder(VideoMeta)
    };

    /**
     * Defines the data and procedure for decoding a single frame of video.
     */
    descriptor VideoDecodeOperation {
        enum Flags {
            EMPTY = 0,
            SESSION_RESET = bit(0)
        };
        uint32_t flags = Flags::EMPTY;
        const GPUBuffer *stream = nullptr;
        size_t streamOffset = 0;
        size_t streamSize = 0;
        VideoFrameType frameType = VideoFrameType::INTRA;
        uint32_t referencePriority = 0;
        int decodedFrameIndex = 0;

        const void *sliceHeader = nullptr;
        const void *parameterSet = nullptr;
        const void *sequenceSet = nullptr;

        int pictureOrder[2] = {};
        uint32_t currentDecodeSlot = 0;
        uint8_t decodeReferences = 0;
        const uint8_t *decodeReferenceSlots = nullptr;
        const int *decodePictureOrder = nullptr;
        const int *decodeFrameNumber = nullptr;
        const Texture *decodeBuffer = nullptr;
    };

    /**
     * Images used in a render pass need some extra metadata about them.
     */
    descriptor RenderPassImage {
        enum class Type {
            RENDER_TARGET,
            DEPTH_STENCIL,
            RESOLVE,
            RESOLVE_DEPTH,
            SHADING_RATE_SOURCE
        } type = Type::RENDER_TARGET;

        enum class LoadOp {
            LOAD,
            CLEAR,
            DONTCARE
        } loadOp = LoadOp::LOAD;

        const Texture *texture = nullptr;

        int subResource = -1;

        enum class StoreOp {
            STORE,
            DONTCARE
        } storeOp = StoreOp::STORE;

        ResourceState before = ResourceState::UNDEFINED;
        ResourceState during = ResourceState::UNDEFINED;
        ResourceState after = ResourceState::UNDEFINED;

        enum class DepthResolveMode {
            MIN,
            MAX
        } depthResolve = DepthResolveMode::MIN;

        static RenderPassImage
        RenderTarget(const Texture *tex, LoadOp loadOp = LoadOp::LOAD, StoreOp storeOp = StoreOp::STORE,
                     ResourceState before = ResourceState::SHADER_RESOURCE,
                     ResourceState after = ResourceState::SHADER_RESOURCE, int subResource = -1) {
            RenderPassImage img{
                    .type = Type::RENDER_TARGET,
                    .loadOp = loadOp,
                    .texture = tex,
                    .subResource = subResource,
                    .storeOp = storeOp,
                    .before = before,
                    .during = ResourceState::RENDER_TARGET,
                    .after = after,
            };

            return img;
        }

        static RenderPassImage
        DepthStencil(const Texture *tex, LoadOp loadOp = LoadOp::LOAD, StoreOp storeOp = StoreOp::STORE,
                     ResourceState before = ResourceState::DEPTH_STENCIL,
                     ResourceState during = ResourceState::DEPTH_STENCIL,
                     ResourceState after = ResourceState::DEPTH_STENCIL, int subResource = -1) {
            RenderPassImage img{
                    .type = Type::DEPTH_STENCIL,
                    .loadOp = loadOp,
                    .texture = tex,
                    .subResource = subResource,
                    .storeOp = storeOp,
                    .before = before,
                    .during = during,
                    .after = after,
            };

            return img;
        }

        static RenderPassImage Resolve(const Texture *tex, ResourceState before = ResourceState::SHADER_RESOURCE,
                                       ResourceState after = ResourceState::SHADER_RESOURCE, int subResource = -1) {
            RenderPassImage img{
                    .type = Type::RESOLVE,
                    .texture = tex,
                    .subResource = subResource,
                    .before = before,
                    .during = ResourceState::COPY_DESTINATION,
                    .after = after,
            };

            return img;
        }

        static RenderPassImage ResolveDepth(const Texture *tex, DepthResolveMode mode = DepthResolveMode::MIN,
                                            ResourceState before = ResourceState::SHADER_RESOURCE,
                                            ResourceState after = ResourceState::SHADER_RESOURCE,
                                            int subResource = -1) {
            RenderPassImage img{
                    .type = Type::RESOLVE_DEPTH,
                    .texture = tex,
                    .subResource = subResource,
                    .before = before,
                    .during = ResourceState::COPY_DESTINATION,
                    .after = after,
                    .depthResolve = mode,
            };

            return img;
        }

        static RenderPassImage
        ShadingRateSource(const Texture *tex, ResourceState before = ResourceState::SHADING_RATE_SOURCE,
                          ResourceState after = ResourceState::SHADING_RATE_SOURCE) {
            RenderPassImage img{
                    .type = Type::SHADING_RATE_SOURCE,
                    .texture = tex,
                    .before = before,
                    .during = ResourceState::SHADING_RATE_SOURCE,
                    .after = after,
            };

            return img;
        }

    };

    /**
     * When creating or updating a Render Pass, we need to tell it all kinds of information about how it's going to be dealing with images.
     */
    descriptor RenderPassMeta {
        ImageFormat targetFormats[8] = {};
        uint32_t targetCount = 0;
        ImageFormat depthStencilFormat = ImageFormat::UNKNOWN;
        uint32_t sampleCount = 0;

        /**
         * Convert all of the formats of the render targets, the depth stencil, and the sample count to an integer.
         * If the hash of the current render pass matches the hash of the one we're trying to swap it to... we don't need to change anything!
         */
        constexpr size_t getHash() const {
            union Hasher {
                struct {
                    size_t format0: 6;
                    size_t format1: 6;
                    size_t format2: 6;
                    size_t format3: 6;
                    size_t format4: 6;
                    size_t format5: 6;
                    size_t format6: 6;
                    size_t format7: 6;
                    size_t depthFormat: 6;
                    size_t sampleCount: 3;
                } bits;
                size_t value;
            } hasher = {
                    .bits = {
                            .format0 = (size_t) targetFormats[0],
                            .format1 = (size_t) targetFormats[1],
                            .format2 = (size_t) targetFormats[2],
                            .format3 = (size_t) targetFormats[3],
                            .format4 = (size_t) targetFormats[4],
                            .format5 = (size_t) targetFormats[5],
                            .format6 = (size_t) targetFormats[6],
                            .format7 = (size_t) targetFormats[7],
                            .depthFormat = (size_t) depthStencilFormat,
                            .sampleCount = (size_t) sampleCount,
                    }
            };

            return hasher.value;
        }

        static constexpr RenderPassMeta from(const RenderPassImage *images, uint32_t imageCount) {
            RenderPassMeta meta;
            for (size_t i = 0; i < imageCount; i++) {
                const RenderPassImage &image = images[i];
                const TextureMeta &desc = image.texture->getMeta();
                switch (image.type) {
                    case RenderPassImage::Type::RENDER_TARGET:
                        meta.targetFormats[meta.targetCount++] = desc.format;
                        meta.sampleCount = desc.sampleCount;
                        break;
                    case RenderPassImage::Type::DEPTH_STENCIL:
                        meta.depthStencilFormat = desc.format;
                        meta.sampleCount = desc.sampleCount;
                        break;
                    default:
                        break;
                }
            }

            return meta;
        }

        static constexpr RenderPassMeta from(const SwapchainMeta &swapchainImages) {
            RenderPassMeta meta;
            meta.targetFormats[0] = swapchainImages.format;
            meta.targetCount = 1;
            return meta;
        }
    };

    /**
     * A heap of Query Objects - allows repeatedly asking the GPU to report a certain statistic.
     */
    graphicsItem(GPUQueryHeap) {
        metaHolder(GPUQueryHeapMeta)
    };

    /**
     * PSO - Pipeline State Object.
     * This must be changed to begin rendering a new material type.
     */
    graphicsItem(PipelineState) {
        metaHolder(PipelineStateMeta)
    };

    /**
     * Swapchain - handles double/triple buffering by automatically switching out the render target at the end of a frame.
     */
    graphicsItem(SwapChain) {
        metaHolder(SwapchainMeta)
    };
}

// Ray Tracing
namespace rx {

    /**
     * When dealing with Ray Tracing Acceleration Structures, there's a few common items that can be defined..
     */
    descriptor RaytracingAccelerationMeta {
        bitfield Flags {
            EMPTY = 0,
            ALLOW_UPDATE = bit(0), // Whether the structure is mutable
            ALLOW_COMPATCTION = bit(1), // Whether the structure must keep its alignment
            PREFER_FAST_TRACE = bit(2), // Sacrifices detail for speed
            PREFER_FAST_BUILD = bit(3), // Sacrifices detail for first-time scene build
            MINIMIZE_MEMORY = bit(4),   // Set up the structure so that the ray tracer uses as little GPU memory as possible.
        } flags = Flags::EMPTY;

        enum class Type {
            BOTTOMLEVEL,    // Per-Object
            TOPLEVEL        // Per-World
        } type = Type::BOTTOMLEVEL;

        struct BottomLevel {
            struct Geometry {
                bitfield Flags {
                    EMPTY = 0,
                    OPAQUE = 1,
                    NO_DUPLICATE_INVOCATION = 2,
                    USE_TRANSFORM = 4
                } flags = Flags::EMPTY;

                enum class Type {
                    TRIANGLES,
                    BOUNDING_BOXES
                } type = Type::TRIANGLES;

                struct Triangles {
                    GPUBuffer vertices;
                    GPUBuffer indexes;
                    uint32_t indexCount = 0;
                    size_t indexOffset = 0;
                    uint32_t vertexCount = 0;
                    size_t vertexOffset = 0;
                    uint32_t vertexStride = 0;
                    IndexBufferFormat idxFormat = IndexBufferFormat::UINT32;
                    ImageFormat vtxFormat = ImageFormat::R32G32B32_FLOAT; // x y z float
                    GPUBuffer transform;
                    uint32_t transformOffset = 0;
                } triangles;

                struct BoundingBoxes {
                    GPUBuffer AABBBuffer;
                    uint32_t offset = 0;
                    uint32_t count = 0;
                    uint32_t stride = 0;
                } boundingBoxes;
            };

            std::vector<Geometry> geometries;
        } bottomLevel;

        struct TopLevel {
            struct Instance {
                enum Flags {
                    EMPTY = 0,
                    TRIANGLE_CULL_DISTANCE = 1,
                    TRIANGLE_REVERSE_WINDING = 2,
                    FORCE_OPAQUE = 4,
                    FORCE_NON_OPAQUE = 8
                };

                float transform[3][4];
                uint32_t instanceID : 24;
                uint32_t instanceMask : 8;
                uint32_t instanceContrib : 24;
                uint32_t flags : 8;
                const GPUResource* bottomLevel = nullptr;
            };

            GPUBuffer instanceBuffer;
            uint32_t offset = 0;
            uint32_t count = 0;
        } topLevel;
    };

    resource(RaytracingAcceleration) {
        metaHolder(RaytracingAccelerationMeta)

        size_t size = 0;
    };

    /**
     * Shaders can be used for picking vertices along certain axes.
     */
    descriptor ShaderLibrary {
        enum class Type {
            RAY_GENERATION,
            MISS,
            CLOSEST_HIT,
            ANY_HIT,
            INTERSECTION
        } type = Type::RAY_GENERATION;

        const Shader* shader = nullptr;
        std::string function;
    };

    /**
     * Shaders are assigned IDs, and some are better used for certain usages, so are recorded here.
     */
    descriptor ShaderHitGroup {
        enum class Type {
            GENERAL,
            TRIANGLES,
            PROCEDURAL
        } type = Type::TRIANGLES;

        std::string name;

        uint32_t generalShader = ~0u;
        uint32_t closestHitShader = ~0u;
        uint32_t anyHitShader = ~0u;
        uint32_t intersectionShader = ~0u;
    };

    /**
     * When setting up a Ray Tracing pipeline, we need to know all of the shaders that will be called. as well as some base properties.
     */
    descriptor RaytracingPipelineMeta {
        std::vector<ShaderLibrary> libraries;
        std::vector<ShaderHitGroup> hitGroups;
        uint32_t maxRecursion = 1;
        uint32_t maxAttributeSize = 0;
        uint32_t maxPayloadSize = 0;
    };

    /**
     * A stand-in for a render pipeline, when using the Ray Tracing backend.
     */
    graphicsItem(RaytracingPipeline) {
        metaHolder(RaytracingPipelineMeta)
    };

    /**
     * An easy way to reference entries in tables output by the Ray Tracing shaders.
     */
    descriptor ShaderTable {
        const GPUBuffer* buffer = nullptr;
        size_t offset = 0;
        size_t size = 0;
        size_t stride = 0;
    };

    /**
     * All of the information required to dispatch any number of rays.
     */
    descriptor RayDispatch {
        ShaderTable rayGeneration;
        ShaderTable miss;
        ShaderTable hitGroup;
        ShaderTable callable;
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
    };
}

// Sparse
namespace rx {
    // Identifies a resource in sparse space
    descriptor SparseResourceCoordinate {
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t z = 0;
        uint32_t mip = 0;
        uint32_t slice = 0;
    };

    // Sparse resources are limited to a specific size in a buffer.
    descriptor SparseRegionSize {
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
    };

    bitfield TileRangeFlags {
        NONE = 0,
        NUL = bit(0)
    };

    descriptor SparseUpdateCommand {
        const GPUResource* sparseResource = nullptr;
        uint32_t numRegions = 0;
        const SparseResourceCoordinate* coordinates = nullptr;
        const SparseRegionSize* sizes = nullptr;
        const GPUBuffer* tilePool = nullptr;
        const TileRangeFlags* flags = nullptr;
        const uint32_t* rangeStartOffsets = nullptr;
        const uint32_t* rangeTileCounts = nullptr;
    };
}

// Helper methods
namespace rx {
    constexpr bool IsFormatSRGB(ImageFormat format) {
        switch (format) {
            case ImageFormat::R8G8B8A8_UNORM_SRGB:
            case ImageFormat::B8G8R8A8_UNORM_SRGB:
            case ImageFormat::BC1_UNORM_SRGB:
            case ImageFormat::BC2_UNORM_SRGB:
            case ImageFormat::BC3_UNORM_SRGB:
            case ImageFormat::BC7_UNORM_SRGB:
                return true;
            default:
                return false;
        }
    }

    constexpr bool IsFormatUnorm(ImageFormat format) {
        switch (format) {
            case ImageFormat::R16G16B16A16_UNORM:
            case ImageFormat::R10G10B10A2_UNORM:
            case ImageFormat::R8G8B8A8_UNORM:
            case ImageFormat::R8G8B8A8_UNORM_SRGB:
            case ImageFormat::B8G8R8A8_UNORM:
            case ImageFormat::B8G8R8A8_UNORM_SRGB:
            case ImageFormat::R16G16_UNORM:
            case ImageFormat::D24_UNORM_S8_UINT:
            case ImageFormat::R8G8_UNORM:
            case ImageFormat::D16_UNORM:
            case ImageFormat::R16_UNORM:
            case ImageFormat::R8_UNORM:
            case ImageFormat::BC1_UNORM:
            case ImageFormat::BC1_UNORM_SRGB:
            case ImageFormat::BC2_UNORM:
            case ImageFormat::BC2_UNORM_SRGB:
            case ImageFormat::BC3_UNORM:
            case ImageFormat::BC3_UNORM_SRGB:
            case ImageFormat::BC4_UNORM:
            case ImageFormat::BC5_UNORM:
            case ImageFormat::BC7_UNORM:
            case ImageFormat::BC7_UNORM_SRGB:
                return true;
            default:
                return false;
        }
    }

    constexpr bool IsFormatBlockCompressed(ImageFormat format) {
        switch (format) {
            case ImageFormat::BC1_UNORM:
            case ImageFormat::BC1_UNORM_SRGB:
            case ImageFormat::BC2_UNORM:
            case ImageFormat::BC2_UNORM_SRGB:
            case ImageFormat::BC3_UNORM:
            case ImageFormat::BC3_UNORM_SRGB:
            case ImageFormat::BC4_UNORM:
            case ImageFormat::BC4_SNORM:
            case ImageFormat::BC5_UNORM:
            case ImageFormat::BC5_SNORM:
            case ImageFormat::BC6H_UF16:
            case ImageFormat::BC6H_SF16:
            case ImageFormat::BC7_UNORM:
            case ImageFormat::BC7_UNORM_SRGB:
                return true;
            default:
                return false;
        }
    }

    constexpr bool IsFormatDepth(ImageFormat format) {
        switch (format) {
            case ImageFormat::D16_UNORM:
            case ImageFormat::D32_FLOAT:
            case ImageFormat::D32_FLOAT_S8X24_UINT:
            case ImageFormat::D24_UNORM_S8_UINT:
                return true;
            default:
                return false;
        }
    }

    constexpr bool IsFormatStencil(ImageFormat format) {
        switch (format) {
            case ImageFormat::D32_FLOAT_S8X24_UINT:
            case ImageFormat::D24_UNORM_S8_UINT:
                return true;
            default:
                return false;
        }
    }

    constexpr uint32_t GetFormatBlockSize(ImageFormat format) {
        if (IsFormatBlockCompressed(format)) {
            return 4u;
        }
        return 1u;
    }

    constexpr uint32_t GetFormatStride(ImageFormat format) {
        switch (format) {
            case ImageFormat::BC1_UNORM:
            case ImageFormat::BC1_UNORM_SRGB:
            case ImageFormat::BC4_SNORM:
            case ImageFormat::BC4_UNORM:
                return 8u;

            case ImageFormat::R32G32B32A32_FLOAT:
            case ImageFormat::R32G32B32A32_UINT:
            case ImageFormat::R32G32B32A32_SINT:
            case ImageFormat::BC2_UNORM:
            case ImageFormat::BC2_UNORM_SRGB:
            case ImageFormat::BC3_UNORM:
            case ImageFormat::BC3_UNORM_SRGB:
            case ImageFormat::BC5_SNORM:
            case ImageFormat::BC5_UNORM:
            case ImageFormat::BC6H_UF16:
            case ImageFormat::BC6H_SF16:
            case ImageFormat::BC7_UNORM:
            case ImageFormat::BC7_UNORM_SRGB:
                return 16u;

            case ImageFormat::R32G32B32_FLOAT:
            case ImageFormat::R32G32B32_UINT:
            case ImageFormat::R32G32B32_SINT:
                return 12u;

            case ImageFormat::R16G16B16A16_FLOAT:
            case ImageFormat::R16G16B16A16_UNORM:
            case ImageFormat::R16G16B16A16_UINT:
            case ImageFormat::R16G16B16A16_SNORM:
            case ImageFormat::R16G16B16A16_SINT:
                return 8u;

            case ImageFormat::R32G32_FLOAT:
            case ImageFormat::R32G32_UINT:
            case ImageFormat::R32G32_SINT:
            case ImageFormat::D32_FLOAT_S8X24_UINT:
                return 8u;

            case ImageFormat::R10G10B10A2_UNORM:
            case ImageFormat::R10G10B10A2_UINT:
            case ImageFormat::R11G11B10_FLOAT:
            case ImageFormat::R8G8B8A8_UNORM:
            case ImageFormat::R8G8B8A8_UNORM_SRGB:
            case ImageFormat::R8G8B8A8_UINT:
            case ImageFormat::R8G8B8A8_SNORM:
            case ImageFormat::R8G8B8A8_SINT:
            case ImageFormat::B8G8R8A8_UNORM:
            case ImageFormat::B8G8R8A8_UNORM_SRGB:
            case ImageFormat::R16G16_FLOAT:
            case ImageFormat::R16G16_UNORM:
            case ImageFormat::R16G16_UINT:
            case ImageFormat::R16G16_SNORM:
            case ImageFormat::R16G16_SINT:
            case ImageFormat::D32_FLOAT:
            case ImageFormat::R32_FLOAT:
            case ImageFormat::R32_UINT:
            case ImageFormat::R32_SINT:
            case ImageFormat::D24_UNORM_S8_UINT:
            case ImageFormat::R9G9B9E5_SHARED:
                return 4u;

            case ImageFormat::R8G8_UNORM:
            case ImageFormat::R8G8_UINT:
            case ImageFormat::R8G8_SNORM:
            case ImageFormat::R8G8_SINT:
            case ImageFormat::R16_FLOAT:
            case ImageFormat::D16_UNORM:
            case ImageFormat::R16_UNORM:
            case ImageFormat::R16_UINT:
            case ImageFormat::R16_SNORM:
            case ImageFormat::R16_SINT:
                return 2u;

            case ImageFormat::R8_UNORM:
            case ImageFormat::R8_UINT:
            case ImageFormat::R8_SNORM:
            case ImageFormat::R8_SINT:
                return 1u;


            default:
                assert(0); // didn't catch format!
                return 16u;
        }
    }

    constexpr ImageFormat GetFormatNonSRGB(ImageFormat format) {
        switch (format) {
            case ImageFormat::R8G8B8A8_UNORM_SRGB:
                return ImageFormat::R8G8B8A8_UNORM;
            case ImageFormat::B8G8R8A8_UNORM_SRGB:
                return ImageFormat::B8G8R8A8_UNORM;
            case ImageFormat::BC1_UNORM_SRGB:
                return ImageFormat::BC1_UNORM;
            case ImageFormat::BC2_UNORM_SRGB:
                return ImageFormat::BC2_UNORM;
            case ImageFormat::BC3_UNORM_SRGB:
                return ImageFormat::BC3_UNORM;
            case ImageFormat::BC7_UNORM_SRGB:
                return ImageFormat::BC7_UNORM;
            default:
                return format;
        }
    }

    constexpr ImageFormat GetFormatSRGB(ImageFormat format) {
        switch (format) {
            case ImageFormat::R8G8B8A8_UNORM:
            case ImageFormat::R8G8B8A8_UNORM_SRGB:
                return ImageFormat::R8G8B8A8_UNORM_SRGB;
            case ImageFormat::B8G8R8A8_UNORM:
            case ImageFormat::B8G8R8A8_UNORM_SRGB:
                return ImageFormat::B8G8R8A8_UNORM_SRGB;
            case ImageFormat::BC1_UNORM:
            case ImageFormat::BC1_UNORM_SRGB:
                return ImageFormat::BC1_UNORM_SRGB;
            case ImageFormat::BC2_UNORM:
            case ImageFormat::BC2_UNORM_SRGB:
                return ImageFormat::BC2_UNORM_SRGB;
            case ImageFormat::BC3_UNORM:
            case ImageFormat::BC3_UNORM_SRGB:
                return ImageFormat::BC3_UNORM_SRGB;
            case ImageFormat::BC7_UNORM:
            case ImageFormat::BC7_UNORM_SRGB:
                return ImageFormat::BC7_UNORM_SRGB;
            default:
                return ImageFormat::UNKNOWN;
        }
    }

    constexpr const char* GetFormatString(ImageFormat format) {
        switch (format) {
            case ImageFormat::UNKNOWN:
                return "UNKNOWN";
            case ImageFormat::R32G32B32A32_FLOAT:
                return "R32G32B32A32_FLOAT";
            case ImageFormat::R32G32B32A32_UINT:
                return "R32G32B32A32_UINT";
            case ImageFormat::R32G32B32A32_SINT:
                return "R32G32B32A32_SINT";
            case ImageFormat::R32G32B32_FLOAT:
                return "R32G32B32_FLOAT";
            case ImageFormat::R32G32B32_UINT:
                return "R32G32B32_UINT";
            case ImageFormat::R32G32B32_SINT:
                return "R32G32B32_SINT";
            case ImageFormat::R16G16B16A16_FLOAT:
                return "R16G16B16A16_FLOAT";
            case ImageFormat::R16G16B16A16_UNORM:
                return "R16G16B16A16_UNORM";
            case ImageFormat::R16G16B16A16_UINT:
                return "R16G16B16A16_UINT";
            case ImageFormat::R16G16B16A16_SNORM:
                return "R16G16B16A16_SNORM";
            case ImageFormat::R16G16B16A16_SINT:
                return "R16G16B16A16_SINT";
            case ImageFormat::R32G32_FLOAT:
                return "R32G32_FLOAT";
            case ImageFormat::R32G32_UINT:
                return "R32G32_UINT";
            case ImageFormat::R32G32_SINT:
                return "R32G32_SINT";
            case ImageFormat::D32_FLOAT_S8X24_UINT:
                return "D32_FLOAT_S8X24_UINT";
            case ImageFormat::R10G10B10A2_UNORM:
                return "R10G10B10A2_UNORM";
            case ImageFormat::R10G10B10A2_UINT:
                return "R10G10B10A2_UINT";
            case ImageFormat::R11G11B10_FLOAT:
                return "R11G11B10_FLOAT";
            case ImageFormat::R8G8B8A8_UNORM:
                return "R8G8B8A8_UNORM";
            case ImageFormat::R8G8B8A8_UNORM_SRGB:
                return "R8G8B8A8_UNORM_SRGB";
            case ImageFormat::R8G8B8A8_UINT:
                return "R8G8B8A8_UINT";
            case ImageFormat::R8G8B8A8_SNORM:
                return "R8G8B8A8_SNORM";
            case ImageFormat::R8G8B8A8_SINT:
                return "R8G8B8A8_SINT";
            case ImageFormat::B8G8R8A8_UNORM:
                return "B8G8R8A8_UNORM";
            case ImageFormat::B8G8R8A8_UNORM_SRGB:
                return "B8G8R8A8_UNORM_SRGB";
            case ImageFormat::R16G16_FLOAT:
                return "R16G16_FLOAT";
            case ImageFormat::R16G16_UNORM:
                return "R16G16_UNORM";
            case ImageFormat::R16G16_UINT:
                return "R16G16_UINT";
            case ImageFormat::R16G16_SNORM:
                return "R16G16_SNORM";
            case ImageFormat::R16G16_SINT:
                return "R16G16_SINT";
            case ImageFormat::D32_FLOAT:
                return "D32_FLOAT";
            case ImageFormat::R32_FLOAT:
                return "R32_FLOAT";
            case ImageFormat::R32_UINT:
                return "R32_UINT";
            case ImageFormat::R32_SINT:
                return "R32_SINT";
            case ImageFormat::D24_UNORM_S8_UINT:
                return "D24_UNORM_S8_UINT";
            case ImageFormat::R9G9B9E5_SHARED:
                return "R9G9B9E5_SHAREDEXP";
            case ImageFormat::R8G8_UNORM:
                return "R8G8_UNORM";
            case ImageFormat::R8G8_UINT:
                return "R8G8_UINT";
            case ImageFormat::R8G8_SNORM:
                return "R8G8_SNORM";
            case ImageFormat::R8G8_SINT:
                return "R8G8_SINT";
            case ImageFormat::R16_FLOAT:
                return "R16_FLOAT";
            case ImageFormat::D16_UNORM:
                return "D16_UNORM";
            case ImageFormat::R16_UNORM:
                return "R16_UNORM";
            case ImageFormat::R16_UINT:
                return "R16_UINT";
            case ImageFormat::R16_SNORM:
                return "R16_SNORM";
            case ImageFormat::R16_SINT:
                return "R16_SINT";
            case ImageFormat::R8_UNORM:
                return "R8_UNORM";
            case ImageFormat::R8_UINT:
                return "R8_UINT";
            case ImageFormat::R8_SNORM:
                return "R8_SNORM";
            case ImageFormat::R8_SINT:
                return "R8_SINT";
            case ImageFormat::BC1_UNORM:
                return "BC1_UNORM";
            case ImageFormat::BC1_UNORM_SRGB:
                return "BC1_UNORM_SRGB";
            case ImageFormat::BC2_UNORM:
                return "BC2_UNORM";
            case ImageFormat::BC2_UNORM_SRGB:
                return "BC2_UNORM_SRGB";
            case ImageFormat::BC3_UNORM:
                return "BC3_UNORM";
            case ImageFormat::BC3_UNORM_SRGB:
                return "BC3_UNORM_SRGB";
            case ImageFormat::BC4_UNORM:
                return "BC4_UNORM";
            case ImageFormat::BC4_SNORM:
                return "BC4_SNORM";
            case ImageFormat::BC5_UNORM:
                return "BC5_UNORM";
            case ImageFormat::BC5_SNORM:
                return "BC5_SNORM";
            case ImageFormat::BC6H_UF16:
                return "BC6H_UF16";
            case ImageFormat::BC6H_SF16:
                return "BC6H_SF16";
            case ImageFormat::BC7_UNORM:
                return "BC7_UNORM";
            case ImageFormat::BC7_UNORM_SRGB:
                return "BC7_UNORM_SRGB";
            case ImageFormat::NV12:
                return "NV12";
            default:
                return "";
        }
    }

    constexpr IndexBufferFormat GetIndexBufferFormat(ImageFormat format) {
        switch (format) {
            default:
            case ImageFormat::R32_UINT:
                return IndexBufferFormat::UINT32;
            case ImageFormat::R16_UINT:
                return IndexBufferFormat::UINT16;
        }
    }

    constexpr IndexBufferFormat GetIndexBufferFormat(uint32_t vertex_count) {
        return vertex_count > 65536 ? IndexBufferFormat::UINT32 : IndexBufferFormat::UINT16;
    }

    constexpr ImageFormat GetIndexBufferFormatRaw(uint32_t vertex_count) {
        return vertex_count > 65536 ? ImageFormat::R32_UINT : ImageFormat::R16_UINT;
    }

    constexpr const char* GetIndexBufferFormatString(IndexBufferFormat format) {
        switch (format) {
            default:
            case IndexBufferFormat::UINT32:
                return "UINT32";
            case IndexBufferFormat::UINT16:
                return "UINT16";
        }
    }

    constexpr const char GetComponentSwizzleChar(ComponentSwizzle value) {
        switch (value) {
            default:
            case ComponentSwizzle::R:
                return 'R';
            case ComponentSwizzle::G:
                return 'G';
            case ComponentSwizzle::B:
                return 'B';
            case ComponentSwizzle::A:
                return 'A';
            case ComponentSwizzle::ZERO:
                return '0';
            case ComponentSwizzle::ONE:
                return '1';
        }
    }

    struct SwizzleString {
        char chars[5] = {};

        constexpr operator const char *() const { return chars; }
    };

    constexpr const SwizzleString GetSwizzleString(Swizzle swizzle) {
        SwizzleString ret;
        ret.chars[0] = GetComponentSwizzleChar(swizzle.r);
        ret.chars[1] = GetComponentSwizzleChar(swizzle.g);
        ret.chars[2] = GetComponentSwizzleChar(swizzle.b);
        ret.chars[3] = GetComponentSwizzleChar(swizzle.a);
        ret.chars[4] = 0;
        return ret;
    }

    template<typename T>
    constexpr T AlignTo(T value, T alignment) {
        return ((value + alignment - T(1)) / alignment) * alignment;
    }
    template<typename T>
    constexpr bool IsAligned(T value, T alignment) {
        return value == AlignTo(value, alignment);
    }

    constexpr uint32_t GetMipCount(uint32_t width, uint32_t height, uint32_t depth = 1u, uint32_t min_dimension = 1u, uint32_t required_alignment = 1u) {
        uint32_t mips = 1;
        while (width > min_dimension || height > min_dimension || depth > min_dimension) {
            width = std::max(min_dimension, width >> 1u);
            height = std::max(min_dimension, height >> 1u);
            depth = std::max(min_dimension, depth >> 1u);
            if ( AlignTo(width, required_alignment) != width ||
                 AlignTo(height, required_alignment) != height ||
                 AlignTo(depth, required_alignment) != depth )
                break;
            mips++;
        }
        return mips;
    }
    constexpr size_t ComputeTextureMemorySizeInBytes(const TextureMeta& desc) {
        size_t size = 0;
        const uint32_t bytes_per_block = GetFormatStride(desc.format);
        const uint32_t pixels_per_block = GetFormatBlockSize(desc.format);
        const uint32_t num_blocks_x = desc.width / pixels_per_block;
        const uint32_t num_blocks_y = desc.height / pixels_per_block;
        const uint32_t mips = desc.mipLevels == 0 ? GetMipCount(desc.width, desc.height, desc.depth) : desc.mipLevels;
        for (uint32_t layer = 0; layer < desc.arraySize; ++layer) {
            for (uint32_t mip = 0; mip < mips; ++mip) {
                const uint32_t width = std::max(1u, num_blocks_x >> mip);
                const uint32_t height = std::max(1u, num_blocks_y >> mip);
                const uint32_t depth = std::max(1u, desc.depth >> mip);
                size += width * height * depth * bytes_per_block;
            }
        }
        size *= desc.sampleCount;
        return size;
    }
}


#undef bitfield
#undef bit

#undef descriptor

#undef graphicsItem
#undef resource

#undef metaHolder

template<>
struct enable_bitmask_operators<rx::BindFlag> {
    static const bool enable = true;
};

template<>
struct enable_bitmask_operators<rx::GraphicsDeviceCapability> {
    static const bool enable = true;
};

template<>
struct enable_bitmask_operators<rx::ResourceFlags> {
  static const bool enable = true;
};
