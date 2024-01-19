#include "../inc/TestScene.h"
#include "shadow/entitiy/entities/NullActor.h"
#include "shadow/entitiy/entities/Position.h"
#include "shadow/entitiy/entities/MeshComponent.h"
#include "shadow/entitiy/entities/Light.h"
#include "shadow/core/ShadowApplication.h"
#include "shadow/renderer/vlkx/render/render_pass/ScreenRenderPass.h"

SHObject_Base_Impl(TestScene)

void TestScene::Build() {
    using ShadowEngine::Assets::Mesh;
    //Add 2 nodes to the scene that have a position and mesh components on them

    std::shared_ptr<vlkxtemp::Model> model;
    std::shared_ptr<vlkx::PushConstant> push;

    push = std::make_unique<vlkx::PushConstant>(
        sizeof(ShadowEngine::Entities::Builtin::MeshComponent::Transformation), 2);

    model = vlkxtemp::ModelBuilder{
        "Walrus", 2, 16.0f/9.0f,
        vlkxtemp::ModelBuilder::SingleMeshModel{"resources/walrus/cube.obj", 1,
                                                {{vlkxtemp::ModelBuilder::TextureType::Diffuse,
                                                  {{"resources/walrus/texture.png"}}}}
        }}
        .bindTextures(vlkxtemp::ModelBuilder::TextureType::Diffuse, 1)
        .pushStage(VK_SHADER_STAGE_VERTEX_BIT)
        .pushConstant(push.get(), 0)
        .shader(VK_SHADER_STAGE_VERTEX_BIT, "resources/walrus/cube.vert.spv")
        .shader(VK_SHADER_STAGE_FRAGMENT_BIT, "resources/walrus/cube.frag.spv")
        .build();

    auto rendererPtr = SH::ShadowApplication::Get().GetModuleManager().GetById<SH::RendererModule>(
        "module:/renderer/vulkan");
    auto renderer = rendererPtr.lock();
    auto extent = renderer->GetRenderExtent();
    model->update(true,
                  extent,
                  VK_SAMPLE_COUNT_1_BIT,
                  *VulkanModule::getInstance()->getRenderPass()->getPass(),
                  0);

    using namespace ShadowEngine::Entities::Builtin;

    MeshComponent component(model, push);

    auto cube1 = this->Add<SH::Entities::Builtin::NullActor>({});
    cube1->SetName("Cube 1");
    cube1->Add<SH::Entities::Builtin::Position>({-5, 0, 0});
    cube1->Add<MeshComponent>(component);

    auto cube2 = this->Add<SH::Entities::Builtin::NullActor>({});
    cube2->SetName("Cube 2");
    cube2->Add<SH::Entities::Builtin::Position>({5, 0, 0});
    cube2->Add<MeshComponent>(component);

    //Add a light to the center of the scene
    auto light = this->Add<SH::Entities::Builtin::NullActor>({});
    light->SetName("Light");
    light->Add<SH::Entities::Builtin::Position>({0, 0, 0});
    light->Add<SH::Entities::Builtin::Light>({});
}