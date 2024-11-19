#include "shadow/renderer/ImGuiModule.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <shadow/core/module-manager-v2.h>
#include <shadow/core/SDL2Module.h>
#include <shadow/core/ShadowApplication.h>
#include <shadow/renderer/IImGuiRenderer.h>
#include <shadow/renderer/RenderManager.h>

#include "shadow/renderer/RendererEvents.h"

namespace SH::Renderer {

    SHObject_Base_Impl(ImGuiModule)
    MODULE_ENTRY(SH::Renderer::ImGuiModule, ImGuiModule)


    void ImGuiModule::PreInit()
    {
        ShadowApplication::Get().GetEventBus().subscribe(this, &ImGuiModule::LateRender);
        ShadowApplication::Get().GetEventBus().subscribe(this, &ImGuiModule::SDLEvent);
    }

    void ImGuiModule::Init()
    {
        if(Mgr().IsModuleActive("module:/render-manager"))
        {
            this->render_mgr = Mgr().GetById<RenderManager>("module:/render-manager");
            const auto render_manager_ptr = this->render_mgr.lock();
            auto render_caps = render_manager_ptr->getRenderer()->getCapabilities();
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

                IImGuiRenderer* renderer = dynamic_cast<IImGuiRenderer*>(render_manager_ptr->getRenderer());
                renderer->InitImGUI();

                spdlog::debug("Imgui Module Init");
            }
            else
            {
                // Unhappy
                // The Renderer can't render ImGUI
            }
        }
        else
        {
            // Unhappy
            // Should not happen as that is a core module and we are dependent on it
        }
    }

    void ImGuiModule::SDLEvent(Events::SDLEvent &e) {
        ImGui_ImplSDL2_ProcessEvent(&e.event);
    }

    void ImGuiModule::LateRender(Renderer::LateRender &e)
    {
        const auto render_mgr = this->render_mgr.lock();
        auto renderer = dynamic_cast<IImGuiRenderer*>(render_mgr->getRenderer());

        renderer->ImGuiFrameStart();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();



        ImGui::ShowDemoWindow();

        ImGui::Render();

        renderer->ImGuiFrameEnd();

        ImGui::EndFrame();



    }
} // Renderer