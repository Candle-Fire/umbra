#include "../../inc/windows/SceneView.h"
#include "imgui.h"
#include "core/ShadowApplication.h"
#include "vlkx/vulkan/VulkanModule.h"
#include "glm/vec2.hpp"
#include "renderer/GameRenderer.h"

namespace ShadowEngine::Editor {

    SHObject_Base_Impl(SceneView)

    std::weak_ptr<vlkx::GameRenderer> renderer;

    SceneView::SceneView() {
        renderer = ShadowEngine::ShadowApplication::Get().GetModuleManager().GetById<vlkx::GameRenderer>("module:/renderer/game");
        auto& output = renderer.lock()->getOutput();
        gameImguiTextures.resize(output.size());
        for (size_t i = 0; i < output.size(); i++) {
            gameImguiTextures[i] = ImGui_ImplVulkan_AddTexture(VkTools::createSampler(VK_FILTER_LINEAR,
                                                                                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                                                      0,
                                                                                      VulkanModule::getInstance()->getDevice()->logical),
                                                               output[i]->getView(),
                                                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    }

    void SceneView::Draw() {
        static bool active = false;
        if (ImGui::Begin("Game View", &active, ImGuiWindowFlags_None)) {

            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            glm::vec2 m_ViewportSize = {viewportPanelSize.x, viewportPanelSize.y};

            ImGui::Image((ImTextureID) gameImguiTextures[0],
                         ImVec2{m_ViewportSize.x, m_ViewportSize.y},
                         ImVec2{0, 0},
                         ImVec2{1, 1});

        }

        ImGui::End();
    }

}