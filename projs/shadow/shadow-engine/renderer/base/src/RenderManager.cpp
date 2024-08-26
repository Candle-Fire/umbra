
#include "shadow/renderer/RenderManager.h"

#include <shadow/core/module-manager-v2.h>

SHObject_Base_Impl(SH::Renderer::RenderManager)

void SH::Renderer::RenderManager::setRenderer(IRenderer* renderer)
{
  this->mainRenderer = renderer;
}

MODULE_ENTRY(SH::Renderer::RenderManager, RenderManager)

