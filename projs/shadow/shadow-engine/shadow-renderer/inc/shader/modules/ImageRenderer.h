#pragma once

#include <shader/ShaderBase.h>

/**
 * Handles the Image Renderer interface with the shader files.
 * Included by both the Image Renderer module and the shaders that it references.
 */

static const uint IMAGE_FLAG_EXTRACT_NORMALMAP = 1u << 0u;
static const uint IMAGE_FLAG_COLORSPACE_HDR = 1u << 1u;
static const uint IMAGE_FLAG_COLORSPACE_LINEAR = 1u << 2u;
static const uint IMAGE_FLAG_FULLSCREEN = 1u << 3u;
static const uint IMAGE_FLAG_MIRRORED = 1u << 4u;
static const uint IMAGE_FLAG_ROUNDED = 1u << 5u;

struct ImageConstants {
    uint flags;
    float hdrScaling;
    uint2 packedColor;
    uint2 texMulAdd;
    uint2 texMulAdd2;

    int bufferIdx;
    uint bufferOffset;
    int samplerIdx;

    int texBaseIdx;
    int texMaskIdx;
    int texBgIdx;

    float soften;
    uint alphaMask;

    uint b0;
    uint b1;
    uint b2;
    uint b3;
};

CONSTANTBUFFER(image, ImageConstants, BUFFER_IMAGE);