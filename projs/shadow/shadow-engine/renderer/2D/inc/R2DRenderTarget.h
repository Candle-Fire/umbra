#pragma once
#include <shadow/renderer/IRenderTarget.h>
#include <VK2D/Structs.h>
#include <VK2D/Texture.h>

namespace SH::Renderer::V2D
{
class R2DTexture final : public IRenderTarget {
  VK2DTexture render_target;
  int width;
  int height;
public:
  explicit R2DTexture(int width, int height)
  {
    render_target = vk2dTextureCreate(width, height);
  }

  ~R2DTexture() override
  {
    vk2dTextureFree(render_target);
  }

  void* GetImageID() override;

  [[nodiscard]] int GetWidth() const
  {
    return width;
  }

  [[nodiscard]] int GetHeight() const
  {
    return height;
  }


  VK2DTexture GetInternalImg()
  {
    return render_target;
  }


};
}
