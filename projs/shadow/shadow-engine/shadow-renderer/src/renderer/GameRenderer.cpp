#include <renderer/GameRenderer.h>
#include "entities/MeshComponent.h"
#include "entities/Position.h"
#include "core/ShadowApplication.h"
#include "core/SDL2Module.h"
#include "EntitySystem.h"

namespace vlkx {

    static std::weak_ptr<ShadowEngine::Entities::EntitySystem> entitySystem;

    SHObject_Base_Impl(GameRenderer)

    MODULE_ENTRY(GameRenderer, GameRenderer)

    void GameRenderer::Init() {
        auto renderer = ShadowEngine::ShadowApplication::Get().GetModuleManager().GetById<VulkanModule>("module:/renderer/vulkan").lock();

        renderPass = std::make_unique<vlkx::ScreenRenderPassManager>(vlkx::RendererConfig { 1, gameOutput, false });
        renderPass->initializeRenderPass();
        renderCommands = std::make_unique<vlkx::RenderCommand>(1);

        gameOutput.resize(1);
        for (size_t i = 0; i < 1; i++) {
            vlkx::TextureImage::Meta meta{
                {nullptr}, {vlkx::ImageUsage::renderTarget(0)}, VK_FORMAT_R8G8B8A8_SRGB, 640, 480, 4
            };
            gameOutput[i] = std::make_unique<vlkx::TextureImage>(0, vlkx::ImageSampler::Config {}, meta);
        }

        gameImguiTextures.resize(gameOutput.size());
        for (size_t i = 0; i < gameOutput.size(); i++) {
            gameImguiTextures[i] = ImGui_ImplVulkan_AddTexture(VkTools::createSampler(VK_FILTER_LINEAR,
                                                                                       VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                                                       0,
                                                                                       renderer->getDevice()->logical),
                                                                                       gameOutput[i]->getView(),
                                                                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        if (entitySystem.expired())
            entitySystem =
                ShadowEngine::ShadowApplication::Get().GetModuleManager().GetById<ShadowEngine::Entities::EntitySystem>("module:/entity-system");
    }

    void GameRenderer::Render() {

        // Collect renderables
        auto& container = *entitySystem.lock()->GetWorld().GetManager().GetContainerByType<ShadowEngine::Entities::Builtin::MeshComponent>();
        ShadowEngine::Entities::rtm_ptr<ShadowEngine::Entities::NodeBase> positionNode;
        ShadowEngine::Entities::Builtin::MeshComponent* mesh;

        renderCommands->executeSimple(1, [](int){ ShadowEngine::ShadowApplication::Get().GetModuleManager().Update(1); },
                                      [&](const VkCommandBuffer &buffer, int frame) {
                                          renderPass->getPass()->execute(buffer, frame, {
                                              [&](const VkCommandBuffer &commands) {

                                                  for (auto & it : container) {
                                                      // Try find the sister position first
                                                      auto parent = *it.GetParent().Get();
                                                      for (auto& comp : parent.GetHierarchy()) {
                                                          if (comp.Get()->GetTypeId() == ShadowEngine::Entities::Builtin::Position::TypeId()) {
                                                              positionNode = comp;
                                                              mesh = &it;

                                                              if (mesh->isMesh) return;

                                                              // Translate by position, submit mesh for render
                                                              auto pos = dynamic_cast<ShadowEngine::Entities::Builtin::Position*>(positionNode.Get());
                                                              *mesh->transform_constant->getData<ShadowEngine::Entities::Builtin::MeshComponent::Transformation>(1) = { glm::translate(glm::identity<glm::mat4>(), { pos->x, pos->y, pos->z } ) };
                                                              mesh->model->draw(commands, 1, 1);
                                                          }
                                                      }
                                                  }
                                              }
                                          });
                                      }
        );
    }
}