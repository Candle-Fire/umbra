#include "core/ShadowApplication.h"
#include "../inc/EditorModule.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>

int main(int argc, char *argv[]) {
    std::cout << "argc == " << argc << '\n';

    for (int ndx{}; ndx != argc; ++ndx) {
        std::cout << "argv[" << ndx << "] == " << std::quoted(argv[ndx]) << '\n';
    }
    std::cout << "argv[" << argc << "] == " << static_cast<void *>(argv[argc]) << '\n';
    /*...*/

    ShadowEngine::ShadowApplication app(argc, argv);
    app.GetModuleManager().AddAssembly({.id="assembly:/shadow-editor", .path="shadow-editor.exe", .type=ShadowEngine::AssemblyType::EXE});
    app.GetModuleManager().LoadModulesFromAssembly("assembly:/shadow-editor");
    app.Init();
    app.Start();

    return argc == 3 ? EXIT_SUCCESS : EXIT_FAILURE; // optional return value
}

extern "C" {
void __declspec(dllexport) assembly_entry(ShadowEngine::ModuleManager &m) {
    m.AddDescriptors({
                         .id="module:/editor",
                         .name = "Editor",
                         .class_name = "EditorModule",
                         .assembly="assembly:/shadow-editor",
                         .dependencies={"module:/platform/sdl2", "module:/renderer/game", "module:/renderer/editor" },
                     }
    );

    m.AddDescriptors({
        .id = "module:/renderer/editor",
        .name = "Editor Window",
        .class_name = "EditorWindow",
        .assembly = "assembly:/shadow-editor",
        .dependencies = { "module:/platform/sdl2",  "module:/renderer/vulkan" }
    });

    }
}