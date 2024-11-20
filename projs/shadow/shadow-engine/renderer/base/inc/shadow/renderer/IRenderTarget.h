#pragma once

namespace SH::Renderer {

  class IRenderTarget {
  public:
    virtual ~IRenderTarget() = default;
    virtual void* GetImageID() = 0;
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
  };
}
