#include <GameModule.h>
#include "spdlog/spdlog.h"
#include "shadow/core/Time.h"
#include "shadow/core/module-manager-v2.h"
#include "shadow/core/ShadowApplication.h"

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

SHObject_Base_Impl(GameModule)

MODULE_ENTRY(GameModule, GameModule);

ConVar var_test("test", 4,"0 = Verbose");

void GameModule::PreInit() {
    spdlog::info("{0} PreInit", this->GetName());
}

void GameModule::Init() {
    spdlog::info("{0} Init", this->GetName());
    spdlog::info("Game Module loading level..");
}
