#include "../../inc/windows/SceneView.h"
#include "imgui.h"
#include "shadow/core/ShadowApplication.h"
//#include "shadow/renderer/vulkan/vlkx/vulkan/VulkanModule.h"
#include <shadow/renderer/RenderManager.h>

#include "glm/vec2.hpp"

namespace SH::Editor {

    SHObject_Base_Impl(SceneView)

    SceneView::SceneView() {
        const auto render_mgr = Mgr().GetById<Renderer::RenderManager>("module:/render-manager").lock();
        renderer = render_mgr->getRenderer();
    }

    void SceneView::Draw() {
        static bool active = false;
        if (ImGui::Begin("Game View", &active, ImGuiWindowFlags_None)) {

            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

            auto& render_target = renderer->getRenderTarget();
            //ImGui::Image((ImTextureID) render_target.GetImageID(),
            //             ImVec2{render_target.GetWidth()+0.0f, render_target.GetHeight()+0.0f},
            //             ImVec2{0, 0},
            //             ImVec2{1, 1});

        }

        ImGui::End();
    }

}