#pragma once

#include "shader/ShaderBase.h"

enum FrameOptions {
    OPTION_TEMPORAL_ANTIALIASING = 1 << 0,
    OPTION_VXGI_ENABLED = 1 << 1,
    OPTION_VXGI_REFLECTIONS = 1 << 2,
    OPTION_SURFELGI_ENABLED = 1 << 3,
    OPTION_REALISTIC_SKY = 1 << 4,
    OPTION_HEIGHT_FOG = 1 << 5,
    OPTION_RT_SHADOWS = 1 << 6,
    OPTION_SHADOW_MASK = 1 << 7,
    OPTION_ALBEDO_DISABLED = 1 << 8,
    OPTION_DIFFUSE_FORCED = 1 << 9,
    OPTION_VOLUMETRIC_CLOUD_CAST_SHADOWS = 1 << 10,
    OPTION_VOLUMETRIC_CLOUD_RECEIVE_SHADOWS = 1 << 11,
    OPTION_FOG_COLOR_OVERRIDE = 1 << 12,
    OPTION_STATIC_SKY = 1 << 13,
    OPTION_REALISTIC_SKY_AERIAL = 1 << 14,
    OPTION_REALISTIC_SKY_HQ = 1 << 15,
    OPTION_REALISTIC_SKY_SHADOWS = 1 << 16,
};

struct alignas(16) ShaderScene {
    int instanceBuf;                                                                                                    // Index of the instance buffer.
    int geometryBuf;                                                                                                    // Index of the geometry buffer.
    int materialBuf;                                                                                                    // Index of the materials buffer.
    int meshBuf;                                                                                                        // Index of the sub-divided mesh buffer.

    int textureStreamBuf;                                                                                               // A buffer used for texture streaming. Will continually change as data is moved back and forth.
    int globalEnvironment;                                                                                              // Index of a texture used for a static sky environment map.
    int globalProbe;                                                                                                    // Index of a fully rendered environment probe.
    int firstBillboard;                                                                                                 // Index of the first billboarded entity.

    int tlas;                                                                                                           // Index of the top-level acceleration structure - the root of the bounding volume hierarchy.
    int rtCount;                                                                                                        // A mutable counter for processing BVHs.
    int rtNodes;                                                                                                        // The number of branch nodes in the acceleration structure.
    int rtPrimitives;                                                                                                   // The number of leaf nodes in the accleration structure.

    float3 AABBMin;                                                                                                     // The lowest set of coordinates of the AABB enclosing the whole scene.
    float padding;                                                                                                      // A padding value to ensure that the coordinates land on a 16 bit boundary.
    float3 AABBMax;                                                                                                     // The highest set of coordinates of the AAB enclosing the whole scene.
    float padding2;                                                                                                     // A padding value to ensure that the coordinates land on a 16 bit boundary.
    float3 AABBSize;                                                                                                    // The size of the AABB enclosing the scene: AABBMax - AABBMin.
    float padding3;                                                                                                     // A padding value to ensure that the coordinates land on a 16 bit boundary.
    float3 AABBInvSize;                                                                                                 // The inverse of the size of the AABB enclosing the scene: 1 / (AABBMax - AABBMin)
    float padding4;                                                                                                     // A padding value to ensure that the coordinates land on a 16 bit boundary.
};

__attribute__((packed)) struct alignas(16) ShaderEntity {
    float3 pos;                                                                                                         // The position of the entity in shader space.
    uint8_t type;                                                                                                       // The type of the entity.
    uint8_t flags;                                                                                                      // Entity flags.
    uint16_t range;                                                                                                     // Visibility range.

    uint direction;                                                                                                     // Facing direction, packed with Cosine cone angle, for directional lighting.

    uint2 color;                                                                                                        // A packed half4, for the color contribution to be passed to the entity's material shaders.

    uint layerMask;                                                                                                     // The render layers that this should be visible from.
    uint indices;                                                                                                       // Indexes of buffers. Packed: Matrices, Textures
    uint remap;                                                                                                         // Depth remapping.
    uint16_t radius;                                                                                                    // Size of the entity's collision, for spherical collision boxes.
    uint16_t length;                                                                                                    // Length of the entity's collision, for cubic collision boxes.

    float4 shadowAtlasFactor;                                                                                           // Contribution factor of this entity to the shadow map.

// Getters for the shaders, setters for the engine
#ifndef __cplusplus
    inline half GetRange() { return (half)f16tof32(range); }
    inline half GetRadius() { return (half)f16tof32(radius); }
    inline half GetLength() { return (half)f16tof32(length); }
    inline half3 GetDirection() {
        return normalize(half3(
            (half)f16tof32(direction.x),
            (half)f16tof32(direction.x >> 16u),
            (half)f16tof32(direction.y)
        ));
    }
    inline half GetConeAngle() { return (half)f16tof32(direction.y >> 16u); }
    inline uint GetShadowCascades() { return (half)f16tof32(direction.y >> 16u); }
    inline half GetAngleScale() { return (half)f16tof32(remap); }
    inline half GetAngleOffset() { return (half)f16tof32(remap >> 16u); }
    inline half GetCubemapDepthRemapNear() { return (half)f16tof32(remap); }
    inline half GetCubemapDepthRemapFar() { return (half)f16tof32(remap >> 16u); }
    inline half4 GetColor() {
        return half4(
            (half)f16tof32(color.x),
            (half)f16tof32(color.x >> 16u),
            (half)f16tof32(color.y),
            (half)f16tof32(color.y >> 16u)
        );
    }
    inline uint GetMatrixIndex() { return indices & 0xFFFF; }
    inline uint GetTextureIndex() { return indices >> 16u; }
    inline bool IsCastingShadow() { return indices != ~0; }
    inline half GetGravity() { return GetConeAngle(); }
    inline float3 GetCollider() { return shadowAtlasFactor.xyz; }
#endif
};

struct alignas(16) FrameConstants {
    uint16_t options;                                                                                                   // Bit packed options from the renderer.
    float time;                                                                                                         // Timestamp of the start of the current frame.
    float timePrev;                                                                                                     // Timestamp of the start of the previous frame.
    float timeDelta;                                                                                                    // time - timePrev

    uint frameIdx;                                                                                                      // The current frame number. Incremented by one for every frame until the program closes.

    uint temporalAARotation;                                                                                            // The factor by which to "rotate" samples used by Temporal AA, to ensure that a representative area of the screen is sampled.

    uint shadowAtlasIdx;                                                                                                // The index of the texture used for the shadow atlas.
    uint shadowAtlasTransparentIdx;                                                                                     // The index of the texture used for the shadow atlas of transparent objects.
    uint2 shadowAtlasExtent;                                                                                            // The size of the shadow atlas texture.

    uint entityCulling;                                                                                                 // The number of entities on screen (that have not been culled).

    ShaderScene scene;                                                                                                  // Information about the scene that the shaders need to use.

    uint envProbes;
    uint dirLights;
    uint spotLights;
    uint pointLights;

    uint lights;
    uint decals;
    uint forces;

    ShaderEntity entities[ENTITY_COUNT];
    float4x4 matrices[ENTITY_COUNT];
};