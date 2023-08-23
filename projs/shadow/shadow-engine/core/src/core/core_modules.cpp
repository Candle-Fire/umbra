#include "core/module-manager-v2.h"

extern "C" {
void assembly_entry(ShadowEngine::ModuleManager &m) {
    m.AddDescriptors({
                         .id="module:/renderer/vulkan",
                         .name = "Vulkan",
                         .class_name = "VulkanModule",
                         .assembly="assembly:/core",
                         .dependencies={"module:/platform/sdl2"},
                     });

    m.AddDescriptors({
        .id = "module:/renderer/game",
        .name = "Game Renderer",
        .class_name = "GameRenderer",
        .assembly = "assembly:/core",
        .dependencies= {
            "module:/platform/sdl2",
            "module:/renderer"
        }
    });

    m.AddDescriptors({
        .id = "module:/renderer",
        .name = "Render Manager",
        .class_name = "RenderOrchestrator",
        .assembly = "assembly:/core",
        .dependencies = {
        }
    });

    m.AddDescriptors({
                         .id="module:/core",
                         .name = "Core",
                         .class_name = "CoreModule",
                         .assembly="assembly:/core",
                     });

    m.AddDescriptors({
                         .id="module:/platform/sdl2",
                         .name = "SDL2",
                         .class_name = "SDL2Module",
                         .assembly="assembly:/core",
                         .dependencies={"module:/core"},
                     });

    m.AddDescriptors({
                         .id="module:/debug",
                         .name = "DebugModule",
                         .class_name = "DebugModule",
                         .assembly="assembly:/core",
                     });

    m.AddDescriptors({
                         .id="module:/entity-system",
                         .name = "EntitySystem",
                         .class_name = "EntitySystem",
                         .assembly="assembly:/core",
                     });
}
}