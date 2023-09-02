#include "shadow/core/module-manager-v2.h"

extern "C" {
[[maybe_unused]] void assembly_entry(SH::ModuleManager &m) {
    m.AddDescriptors({
                         .id="module:/renderer/vulkan",
                         .name = "Vulkan",
                         .class_name = "VulkanModule",
                         .assembly="assembly:/core"_id,
                         .dependencies={"module:/platform/sdl2"},
                     });

    m.AddDescriptors({
                         .id="module:/core",
                         .name = "Core",
                         .class_name = "CoreModule",
                         .assembly="assembly:/core"_id,
                     });

    m.AddDescriptors({
                         .id="module:/platform/sdl2",
                         .name = "SDL2",
                         .class_name = "SDL2Module",
                         .assembly="assembly:/core"_id,
                         .dependencies={"module:/core"},
                     });

    m.AddDescriptors({
                         .id="module:/debug",
                         .name = "DebugModule",
                         .class_name = "DebugModule",
                         .assembly="assembly:/core"_id,
                     });

    m.AddDescriptors({
                         .id="module:/entity-system",
                         .name = "EntitySystem",
                         .class_name = "EntitySystem",
                         .assembly="assembly:/core"_id,
                     });

    m.AddDescriptors({
                         .id="module:/logger",
                         .name = "Logger",
                         .class_name = "LoggerModule",
                         .assembly="assembly:/core"_id,
                     });
}
}