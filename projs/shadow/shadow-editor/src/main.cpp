#include "shadow/core/ShadowApplication.h"
#include "runtime/Runtime.h"

#include "EditorModule.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>

#include <SDL3/SDL_main.h>

int main(int argc, char *argv[]) {
    RUNTIME_BOOT();

    spdlog::debug("CWD: {}", std::filesystem::current_path().string());
    spdlog::debug("argc == {}", argc);
    for (int ndx{}; ndx != argc; ++ndx) {
        spdlog::debug("argv[{}] == {}", ndx, std::string(argv[ndx]));
    }
    spdlog::debug("argv[{}] == {}", argc, static_cast<void *>(argv[argc]));
    /*...*/

    SH::ShadowApplication app(argc, argv);
    app.GetModuleManager().LoadModulesFromAssembly("assembly:/shadow-editor"_id);
    app.Init();
    app.Start();

    return argc == 3 ? EXIT_SUCCESS : EXIT_FAILURE; // optional return value
}

extern "C" {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCInconsistentNamingInspection"
[[maybe_unused]] EXPORT void assembly_entry(SH::ModuleManager &m) {
    m.AddDescriptors({
                         .id="module:/editor",
                         .name = "Editor",
                         .class_name = "EditorModule",
                         .assembly="assembly:/shadow-editor"_id,
                         .dependencies={"module:/platform/sdl3", "module:/renderer/vulkan"},
                     });
}
#pragma clang diagnostic pop
}


