#include <GameModule.h>
#include "imgui.h"
#include "spdlog/spdlog.h"
#include "imgui_impl_vulkan.h"
//#include "imgui_impl_sdl2.h"
#include "shadow/core/Time.h"
#include "shadow/core/module-manager-v2.h"
#include "shadow/core/ShadowApplication.h"
#include "TestScene.h"
#include "shadow/entitiy/EntitySystem.h"

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

SHObject_Base_Impl(GameModule)

MODULE_ENTRY(GameModule, GameModule);

void GameModule::PreInit() {
    spdlog::info("{0} PreInit", this->GetName());
}

void GameModule::Init() {
    spdlog::info("{0} Init", this->GetName());
    spdlog::info("Game Module loading level..");
}
