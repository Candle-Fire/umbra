#include <GameModule.h>
#include "imgui.h"
#include "spdlog/spdlog.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl2.h"
#include "core/Time.h"
#include "vlkx/render/Camera.h"
#include "vlkx/vulkan/abstraction/Buffer.h"
#include "vlkx/render/render_pass/ScreenRenderPass.h"
#include "temp/model/Builder.h"
#include "core/module-manager-v2.h"
#include "core/ShadowApplication.h"
#include "TestScene.h"
#include "EntitySystem.h"

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

SHObject_Base_Impl(GameModule)

MODULE_ENTRY(GameModule, GameModule)

struct Transformation {
    alignas(sizeof(glm::mat4)) glm::mat4 proj_view_model;
};

void GameModule::PreInit() {
    spdlog::info("{0} PreInit", this->GetName());
}

void GameModule::Init() {
    spdlog::info("{0} Init", this->GetName());
    spdlog::info("Game Module loading level..");

    auto w_ensys = ShadowEngine::ShadowApplication::Get().GetModuleManager()
        .GetById<ShadowEngine::Entities::EntitySystem>("module:/entity-system");

    if (w_ensys.expired())
        return;
    auto ensys = w_ensys.lock();
    ensys->GetWorld().AddScene(TestScene());
}
