#include "Renderer2D.h"

#include <shadow/core/module-manager-v2.h>
#include <shadow/core/ShadowApplication.h>
#include <shadow/renderer/RenderManager.h>

SHObject_Base_Impl(SH::Renderer::V2D::Renderer2D)

MODULE_ENTRY(SH::Renderer::V2D::Renderer2D, Renderer2D)

SH::Renderer::V2D::Renderer2D::Renderer2D()
= default;

SH::Renderer::V2D::Renderer2D::~Renderer2D()
= default;

SH::Renderer::RendererCapabilities SH::Renderer::V2D::Renderer2D::getCapabilities() const
{
    return RendererCapabilities::RENDERER_2D | RendererCapabilities::RENDERER_IMGUI;
}

void SH::Renderer::V2D::Renderer2D::setRenderExtent(int width, int height)
{
}

void SH::Renderer::V2D::Renderer2D::RenderSceneToTexture()
{
}

void SH::Renderer::V2D::Renderer2D::BeginRenderPass()
{
}

void SH::Renderer::V2D::Renderer2D::PreInit()
{
    SH::ShadowApplication::Get().GetModuleManager().IfModuleActive("module:/render-manager", [&](RenderManager &rmg)
    {
        rmg.addRenderer(this);
    });
}

void SH::Renderer::V2D::Renderer2D::Update(int frame)
{
    Module::Update(frame);
}
