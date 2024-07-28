#pragma once

#include <Shader/ShaderBase.h>

static const uint FONT_FLAG_SDF_RENDERING = 1u << 0u;
static const uint FONT_FLAG_OUTPUT_HDR10 = 1u << 1u;
static const uint FONT_FLAG_OUTPUT_LINEAR = 1u << 2u;

struct FontVertex {
  float2 pos;
  float2 uv;
};

namespace SDF {
  static const uint padding = 5;
  static const uint edgeValue = 127;
  static const float edgeValueUnorm = float(edgeValue) / 255.0f;
  static const float pixelDistanceScale = float(edgeValue) / float(padding);
}

struct FontConstants {
  int bufferIndex;
  uint bufferOffset;
  int textureIndex;
  int padding;
  float4 color;
  float softness;
  float bolden;
  uint flags;
  float hdrScale;

  float4x4 transform;
};

CONSTANTBUFFER(font, FontConstants, CBSLOT_FONT);