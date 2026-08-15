#include "shadow/core/ShadowApplication.h"
#include "shadow/exports.h"
#include "shadow/core/convar.h"

ConVar *firstVarInAssembly;

extern "C" {
[[maybe_unused]] void EXPORT assembly_entry(SH::ModuleManager &m) {

    ConVarManager::Get()->AddConVars(firstVarInAssembly);

    m.AddDescriptors({
                         .id="module:/game",
                         .name = "Test Game",
                         .class_name = "GameModule",
                         .assembly="assembly:/test-game"_id,
                         .dependencies={"module:/platform/sdl2", "module:/entity-system"},
                     });

    m.AddDescriptors({
                         .id="module:/test-module",
                         .name = "Test Module",
                         .class_name = "TestModule",
                         .assembly="assembly:/test-game"_id,
                     });
}
}