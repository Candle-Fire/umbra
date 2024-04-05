#pragma once

#include "DirectXMath.h"
#include "GraphicsDefine.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "Canvas.h"
#include "Interface.h"

namespace rx::Image {
    using namespace DirectX;

    struct RenderMode {
        enum Flags {
            EMPTY = 0,
            RECT = 1,
            RECT2 = 2,
            MIRROR = 4,
            EXTRACT_NORMALS = 8,
            FULLSCREEN = 16,
            BACKGROUND = 32,
            COLORSPACE_HDR = 64,
            COLORSPACE_LINEAR = 128,
            ROUND_CORNERS = 256
        };

        enum class Quality {
            NEAREST,
            LINEAR,
            ANISO,
            SIZE
        };

        uint32_t flags = EMPTY;
        XMFLOAT3 pos = XMFLOAT3(0, 0, 0);
        XMFLOAT2 size = XMFLOAT2(1, 1);
        XMFLOAT2 scale = XMFLOAT2(1, 1);
        XMFLOAT4 color = XMFLOAT4(1, 1, 1, 1);
        XMFLOAT4 drawRect = XMFLOAT4(0, 0, 0, 0);
        XMFLOAT4 drawRect2 = XMFLOAT4(0, 0, 0, 0);
        XMFLOAT2 texOffset = XMFLOAT2(0, 0);
        XMFLOAT2 texOffset2 = XMFLOAT2(0, 0);
        XMFLOAT2 pivot = XMFLOAT2(0, 0);
        float rotation = 0;
        float fade = 0;
        float opacity = 1;
        float HDRScaling = 1;
        float maskLowerLimit = 0;
        float maskUpperLimit = 1;

        XMFLOAT2 corners[4] = {
                XMFLOAT2(0, 0), XMFLOAT2(1, 0),
                XMFLOAT2(0, 1), XMFLOAT2(1, 1)
        };
        const glm::mat4* customRotate = nullptr;
        const glm::mat4* customProjection = nullptr;

        struct RoundingMode {
            float radius = 0;
            int segments = 18;
        } cornerRounding[4];

        uint8_t stencil = 0;
        rx::ComparisonFunc stencilCompare = ComparisonFunc::NEVER;
        rx::defs::StencilMask stencilMask = defs::StencilMask::ALL;
        rx::defs::BlendModes stencilBlend = defs::BlendModes::ALPHA;
        rx::ImageTiling sampleTiling = ImageTiling::MIRROR;
        Quality quality = Quality::NEAREST;

        const Texture* maskMap = nullptr;
        int imageSubResource = -1;
        int maskSubResource = -1;

        constexpr void setMaskMap(const Texture* tex) { maskMap = tex; }
        constexpr bool isDrawRect() const { return flags & RECT; }
        constexpr bool isDrawRect2() const { return flags & RECT2; }
        constexpr bool isMirror() const { return flags & MIRROR; }
        constexpr bool isExtractNormalMap() const { return flags & EXTRACT_NORMALS; }
        constexpr bool isFullscreen() const { return flags & FULLSCREEN; }
        constexpr bool isBackground() const { return flags & BACKGROUND; }
        constexpr bool isHDR() const { return flags & COLORSPACE_HDR; }
        constexpr bool isLinear() const { return flags & COLORSPACE_LINEAR; }
        constexpr bool isRounded() const { return flags & ROUND_CORNERS; }

        constexpr void enableDrawRect(const XMFLOAT4& rect) { flags |= RECT; drawRect = rect; }
        constexpr void enableDrawRect2(const XMFLOAT4& rect) { flags |= RECT2; drawRect = rect; }
        constexpr void enableMirror() { flags |= MIRROR; }
        constexpr void enableExtractNormals() { flags |= EXTRACT_NORMALS; }
        constexpr void enableFullscreen() { flags |= FULLSCREEN; }
        constexpr void enableBackground() { flags |= BACKGROUND; }
        constexpr void enableHDR() { flags |= COLORSPACE_HDR; }
        constexpr void enableLinear(float scaling = 1) { flags |= COLORSPACE_LINEAR; HDRScaling = scaling; }
        constexpr void enableRounding() { flags |= ROUND_CORNERS; }

        constexpr void disableDrawRect() { flags &= ~RECT; }
        constexpr void disableDrawRect2() { flags &= ~RECT2; }
        constexpr void disableMirror() { flags &= ~MIRROR; }
        constexpr void disableExtractNormals() { flags &= ~EXTRACT_NORMALS; }
        constexpr void disableFullscreen() { flags &= ~FULLSCREEN; }
        constexpr void disableBackground() { flags &= ~BACKGROUND; }
        constexpr void disableHDR() { flags &= ~COLORSPACE_HDR; }
        constexpr void disableLinear() { flags &= ~COLORSPACE_LINEAR; }
        constexpr void disableRounding() { flags &= ~ROUND_CORNERS; }

        RenderMode() = default;

        RenderMode(float width, float height) : size(width, height) {}

        RenderMode(float x, float y, float width, float height, const XMFLOAT4& color = XMFLOAT4(1, 1, 1, 1)) : pos(x, y, 0), size(width, height), color(color) {}

        RenderMode(const XMFLOAT4& color, rx::defs::BlendModes blend = rx::defs::BlendModes::ALPHA, bool background = false) : color(color), stencilBlend(blend) { if (background) enableBackground(); }
    };

    void SetCanvas(const Canvas& canvas);

    void SetBackground(const Texture& tex);

    void Draw(const Texture* tex, const RenderMode& param, ThreadCommands cmd);

    void Initialize();
}