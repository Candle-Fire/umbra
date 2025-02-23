#include <GameModule.h>
#include "imgui.h"
#include "spdlog/spdlog.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl2.h"
#include "shadow/core/Time.h"
#include "shadow/core/module-manager-v2.h"
#include "shadow/core/ShadowApplication.h"
#include "TestScene.h"
#include "glm/glm.hpp"
#include "shadow/entity/EntitySystem.h"

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

    auto w_ensys = SH::ShadowApplication::Get().GetModuleManager()
        .GetById<SH::Entities::EntitySystem>("module:/entity-system");

    if (w_ensys.expired())
        return;
    auto ensys = w_ensys.lock();
    ensys->GetWorld().AddScene(TestScene());
}
