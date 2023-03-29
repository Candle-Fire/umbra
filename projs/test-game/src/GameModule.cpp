#include <GameModule.h>
#include "imgui.h"
#include "spdlog/spdlog.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl.h"
#include "core/Time.h"
#include "vlkx/render/Camera.h"
#include "vlkx/vulkan/abstraction/Buffer.h"
#include "vlkx/render/render_pass/ScreenRenderPass.h"
#include "temp/model/Builder.h"
#include "core/module-manager-v2.h"
#include "core/ShadowApplication.h"

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

SHObject_Base_Impl(GameModule)

MODULE_ENTRY(GameModule, GameModule)

struct Transformation {
  alignas(sizeof(glm::mat4)) glm::mat4 proj_view_model;
};

std::unique_ptr<vlkx::PushConstant> trans_constant_;
std::unique_ptr<vlkxtemp::Model> cube_model_;
float aspectRatio;

std::shared_ptr<ShadowEngine::RendererModule> renderer;

void GameModule::PreInit() {
    spdlog::info("{0} PreInit", this->GetName());
}

void GameModule::Init() {
    spdlog::info("{0} Init", this->GetName());
    spdlog::info("Game Module loading level..");

    trans_constant_ = std::make_unique<vlkx::PushConstant>(
        sizeof(Transformation), 2);

    auto rendererPtr = ShadowEngine::ShadowApplication::Get().GetModuleManager().GetById<ShadowEngine::RendererModule>(
        "module:/renderer/vulkan");
    if (rendererPtr.expired()) {
        return;
    }
    renderer = rendererPtr.lock();

    auto extent = renderer->GetRenderExtent();
    aspectRatio = (float) extent.width / extent.height;

    /* Model */
    cube_model_ = vlkxtemp::ModelBuilder{
        "Walrus", 2, aspectRatio,
        vlkxtemp::ModelBuilder::SingleMeshModel{"resources/walrus/walrus.obj", 1,
                                                {{vlkxtemp::ModelBuilder::TextureType::Diffuse,
                                                  {{"resources/walrus/texture.png"}}}}
        }}
        .bindTextures(vlkxtemp::ModelBuilder::TextureType::Diffuse, 1)
        .pushStage(VK_SHADER_STAGE_VERTEX_BIT)
        .pushConstant(trans_constant_.get(), 0)
        .shader(VK_SHADER_STAGE_VERTEX_BIT, "resources/walrus/cube.vert.spv")
        .shader(VK_SHADER_STAGE_FRAGMENT_BIT, "resources/walrus/cube.frag.spv")
        .build();

    Recreate();
}

void GameModule::Recreate() {
    auto extent = renderer->GetRenderExtent();
    cube_model_->update(true,
                        extent,
                        VK_SAMPLE_COUNT_1_BIT,
                        *VulkanModule::getInstance()->getRenderPass()->getPass(),
                        0);
}

void GameModule::Update(int frame) {
    const float elapsed_time = Time::timeSinceStart;
    const glm::mat4 model = glm::rotate(glm::mat4{1.0f},
                                        (elapsed_time / 1000 / 2) * glm::radians(90.0f),
                                        glm::vec3{1.0f, 1.0f, 0.0f});
    const glm::mat4 view = glm::lookAt(glm::vec3{3.0f}, glm::vec3{0.0f},
                                       glm::vec3{0.0f, 0.0f, 1.0f});
    const glm::mat4 proj = glm::perspective(
        glm::radians(45.0f), aspectRatio,
        0.1f, 100.0f);
    *trans_constant_->getData<Transformation>(frame) = {proj * view * model};
}

void GameModule::Render(VkCommandBuffer &commands, int frame) {
    cube_model_->draw(commands, frame, 1);
}

void GameModule::OverlayRender() {
    bool active = true;
    if (ImGui::Begin("Game module window", &active, ImGuiWindowFlags_MenuBar))
        ImGui::Text("Such text from curle's branch");
    ImGui::End();

    //bool open = false;
    //ImGui::ShowDemoWindow(&open);
}

void GameModule::AfterFrameEnd() {
    Time::UpdateTime();
}

void GameModule::LateRender(VkCommandBuffer &commands, int frame) {}

void GameModule::PreRender() {}

void GameModule::Destroy() {}

void GameModule::Event(SDL_Event *) {}

