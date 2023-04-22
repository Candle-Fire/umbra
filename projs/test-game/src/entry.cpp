#include "core/ShadowApplication.h"

extern "C" {
void assembly_entry(ShadowEngine::ModuleManager &m) {
    m.AddDescriptors({
                         .id="module:/game",
                         .name = "Test Game",
                         .class_name = "GameModule",
                         .assembly="assembly:/test-game",
                         .dependencies={"module:/platform/sdl2", "module:/entity-system"},
                     });

    m.AddDescriptors({
                         .id="module:/test-module",
                         .name = "Test Module",
                         .class_name = "TestModule",
                         .assembly="assembly:/test-game",
                     });
}
}