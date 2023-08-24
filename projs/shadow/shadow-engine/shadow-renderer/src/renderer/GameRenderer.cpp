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

        gameOutput.resize(1);
        for (size_t i = 0; i < 1; i++) {
            vlkx::TextureImage::Meta meta{
                {nullptr}, {vlkx::ImageUsage::renderTarget(0)}, VK_FORMAT_R8G8B8A8_SRGB, 640, 480, 4
            };
            gameOutput[i] = std::make_unique<vlkx::TextureImage>(0, vlkx::ImageSampler::Config {}, meta);
        }

        renderPass = std::make_unique<vlkx::ScreenRenderPassManager>(vlkx::RendererConfig { 1, gameOutput, false });
        renderPass->initializeRenderPass();
        renderCommands = std::make_unique<vlkx::RenderCommand>(1);

        if (entitySystem.expired())
            entitySystem =
                ShadowEngine::ShadowApplication::Get().GetModuleManager().GetById<ShadowEngine::Entities::EntitySystem>("module:/entity-system");

        ShadowEngine::ShadowApplication::Get().GetEventBus().subscribe(this, &GameRenderer::Render);
    }

    void GameRenderer::Render(SH::Events::Render& render) {

        // Collect renderables
        auto& container = *entitySystem.lock()->GetWorld().GetManager().GetContainerByType<ShadowEngine::Entities::Builtin::MeshComponent>();
        auto& scene = *entitySystem.lock()->GetWorld().GetManager().GetContainerByType<ShadowEngine::Entities::Scene>();
        ShadowEngine::Entities::rtm_ptr<ShadowEngine::Entities::NodeBase> positionNode;
        ShadowEngine::Entities::Builtin::MeshComponent* mesh;

        renderCommands->executeSimple(1, [](int){ ShadowEngine::ShadowApplication::Get().GetModuleManager().Update(1); },
                                      [&](const VkCommandBuffer &buffer, int frame) {
                                          renderPass->getPass()->execute(buffer, frame, {
                                              [&](const VkCommandBuffer &commands) {
                                                  ShadowEngine::ShadowApplication::Get().GetEventBus().fire(SH::Events::Recreate());

                                                  for (auto & it : container) {
                                                      // Try find the sister position first
                                                      auto parent = *it.GetParent().Get();
                                                      for (auto& comp : parent.GetHierarchy()) {
                                                          if (comp.Get()->GetTypeId() == ShadowEngine::Entities::Builtin::Position::TypeId()) {
                                                              positionNode = comp;
                                                              mesh = &it;

                                                              // Translate by position, submit mesh for render
                                                              auto pos = dynamic_cast<ShadowEngine::Entities::Builtin::Position*>(positionNode.Get());
                                                              auto ext = gameOutput[0]->getExtent();
                                                              const glm::mat4 model = glm::translate(glm::mat4{1.0f},
                                                                                                     { pos->x, pos->y, pos->z });
                                                              const glm::mat4 view = glm::lookAt(glm::vec3{3.0f}, glm::vec3{0.0f},
                                                                                                 glm::vec3{0.0f, 0.0f, 1.0f});
                                                              const glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float) ext.width / ext.height, 0.1f, 100.0f);
                                                              *mesh->transform_constant->getData<ShadowEngine::Entities::Builtin::MeshComponent::Transformation>(1) = { model * view * proj };
                                                              mesh->model->draw(commands, 0, 1);
                                                          }
                                                      }
                                                  }
                                              }
                                          });
                                      }
        );
    }
}