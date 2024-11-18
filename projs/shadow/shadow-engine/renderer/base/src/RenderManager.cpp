
#include "shadow/renderer/RenderManager.h"

#include <shadow/core/module-manager-v2.h>

SHObject_Base_Impl(SH::Renderer::RenderManager)

void SH::Renderer::RenderManager::setRenderer(IRenderer* renderer)
{
  this->mainRenderer = renderer;
}

SH::Renderer::IRenderer* SH::Renderer::RenderManager::getRenderer() const
{
  return mainRenderer;
}

MODULE_ENTRY(SH::Renderer::RenderManager, RenderManager)

