#include "shadow/renderer/ImGuiModule.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <shadow/core/module-manager-v2.h>
#include <shadow/core/SDL2Module.h>
#include <shadow/renderer/IImGuiRenderer.h>
#include <shadow/renderer/RenderManager.h>

namespace SH::Renderer {

    SHObject_Base_Impl(ImGuiModule)
    MODULE_ENTRY(SH::Renderer::ImGuiModule, ImGuiModule)


    void ImGuiModule::Init()
    {
        if(Mgr().IsModuleActive("module:/render-manager"))
        {
            auto render_mgr = Mgr().GetById<RenderManager>("module:/render-manager").lock();
            auto render_caps = render_mgr->getRenderer()->getCapabilities();
            if(render_caps & RendererCapabilities::RENDERER_IMGUI)
            {
                // Happy
                // The renderer can draw imgui so we are safe to init it and expect the backend to be handled
                // by the renderer
                ImGui::CreateContext();
                // This is a really bad assumption for now
                // TODO: Add the ability for the windowing backend and the render backend to communicate how to do this
                const auto sdl = Mgr().GetById<SDL2Module>("module:/platform/sdl2").lock();
                ImGui_ImplSDL2_InitForVulkan(sdl->window->sdlWindowPtr);

                IImGuiRenderer* renderer = reinterpret_cast<IImGuiRenderer*>(render_mgr->getRenderer());
                renderer->EnableImGui();
            }
            else
            {
                // Unhappy
            }
        }
        else
        {
            // Unhappy
        }
    }


} // Renderer