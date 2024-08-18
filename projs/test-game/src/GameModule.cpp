#include <algorithm>

#include <shadow/renderer/IRenderer.h>
#include <shadow/renderer/RenderManager.h>
#include "shadow/core/module-manager-v2.h"
#include "shadow/core/ShadowApplication.h"

#include "imgui.h"
#include "spdlog/spdlog.h"

#include <GameModule.h>

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

SHObject_Base_Impl(GameModule)

MODULE_ENTRY(GameModule, GameModule)

void GameModule::PreInit()
{
    spdlog::info("{0} PreInit", this->GetName());

    SH::ShadowApplication::Get().GetModuleManager().IfModuleActive<SH::Renderer::RenderManager>
    ("core:/render-manager", [](SH::Renderer::RenderManager& rmg)
    {
        std::ranges::find_if(rmg.getRenderers(), [](SH::Renderer::IRenderer*& r)
        {
            return (bool)(r->getCapabilities() & SH::Renderer::RendererCapabilities::RENDERER_2D);
        });
    });
}

void GameModule::Init()
{
    spdlog::info("{0} Init", this->GetName());
}
