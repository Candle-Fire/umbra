#include <iostream>

#include "core/ShadowApplication.h"
#include "GameModule.h"
#include "core/ShadowApplication.h"
#include "vlkx/vulkan/VulkanModule.h"
#include "TestModule.h"

/*
extern "C" __declspec(dllexport) void shadow_main(ShadowEngine::ShadowApplication* app) {
    app->GetModuleManager().PushModule(std::make_shared<TestModule>(), "game");
    app->GetModuleManager().PushModule(std::make_shared<GameModule>(), "game");
}
*/
extern "C" {
    void assembly_entry(ShadowEngine::ModuleManager &m) {
        m.AddDescriptors({
                                 .id="module:/game",
                                 .name = "Test Game",
                                 .class_name = "GameModule",
                                 .assembly="assembly:/test-game",
                                 .dependencies={"module:/platform/sdl2"},
                         });

        m.AddDescriptors({
                                 .id="module:/test-module",
                                 .name = "Test Module",
                                 .class_name = "TestModule",
                                 .assembly="assembly:/test-game",
                         });
    }
}