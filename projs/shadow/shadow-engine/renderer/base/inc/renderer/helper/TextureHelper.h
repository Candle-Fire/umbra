#pragma once

#include <renderer/GraphicsDefine.h>
#include <DirectXMath.h>

namespace rx::Helper::Texture {
    void Initialize();

    const rx::Texture* GetLogo();
    const rx::Texture* GetRandom();
    const rx::Texture* GetColorGrade();
    const rx::Texture* GetNormalMap();
    const rx::Texture* GetUINT4();
    const rx::Texture* GetBlueNoise();

    const rx::Texture* GetWhite();
    const rx::Texture* GetBlack();
    const rx::Texture* GetTransparent();
    const rx::Texture* GetColor(DirectX::XMFLOAT4 color);

    bool CreateTexture( rx::Texture& tex, const uint8_t* data, uint32_t width, uint32_t height, rx::ImageFormat format = rx::ImageFormat::R8G8B8A8_UNORM, rx::Swizzle swizzle = {});

    enum class GradientType {
      Linear,
      Circular,
      Angular
    };

    enum class GradientFlags {
      None = 0,
      Inverse = 1 << 0,
      Smooth = 1 << 1
    };

    rx::Texture CreateGradientTexture( GradientType type, uint32_t width, uint32_t height, const DirectX::XMFLOAT2& uvStart = DirectX::XMFLOAT2(0,0), const DirectX::XMFLOAT2& uvEnd = DirectX::XMFLOAT2(1, 0), GradientFlags flags = GradientFlags::None, rx::Swizzle swizzle = { rx::ComponentSwizzle::R, rx::ComponentSwizzle::R, rx::ComponentSwizzle::R, rx::ComponentSwizzle::R });

    rx::Texture CreateCircularGradientTexture(uint32_t width, uint32_t height, const DirectX::XMFLOAT2& dir = DirectX::XMFLOAT2(0,1), bool counterClockwise = false, rx::Swizzle swizzle = { rx::ComponentSwizzle::R, rx::ComponentSwizzle::R, rx::ComponentSwizzle::R, rx::ComponentSwizzle::R });
}

template<>
struct enable_bitmask_operators<rx::Helper::Texture::GradientFlags> {
  static const bool enable = true;
};