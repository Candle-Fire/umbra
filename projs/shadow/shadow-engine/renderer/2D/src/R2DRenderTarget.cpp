//
// Created by dpeter99 on 20/11/24.
//

#include "R2DRenderTarget.h"

#include <VK2D/Opaque.h>

namespace SH::Renderer::V2D
{
  void* R2DTexture::GetImageID()
  {
    return render_target->img->set;
  }
}
