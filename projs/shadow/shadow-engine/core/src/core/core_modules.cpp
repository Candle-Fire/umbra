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
                             .id="module:/core",
                             .name = "Core",
                             .class_name = "",
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
}
}