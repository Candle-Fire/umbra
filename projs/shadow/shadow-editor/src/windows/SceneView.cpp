#include "../../inc/windows/SceneView.h"
#include "imgui.h"
#include "core/ShadowApplication.h"
#include "vlkx/vulkan/VulkanModule.h"
#include "glm/vec2.hpp"

namespace ShadowEngine::Editor {

    SHObject_Base_Impl(SceneView)

    SceneView::SceneView() {
        renderer = ShadowEngine::ShadowApplication::Get().GetModuleManager().GetById<VulkanModule>("module:/renderer/vulkan").lock();
    }

    void SceneView::Draw() {
        static bool active = false;
        if (ImGui::Begin("Game View", &active, ImGuiWindowFlags_None)) {

            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            glm::vec2 m_ViewportSize = {viewportPanelSize.x, viewportPanelSize.y};

            ImGui::Image((ImTextureID) renderer->getEditorRenderPlanes(),
                         ImVec2{m_ViewportSize.x, m_ViewportSize.y},
                         ImVec2{0, 0},
                         ImVec2{1, 1});
        }

        ImGui::End();
    }

}