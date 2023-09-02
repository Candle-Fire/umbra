#include "shadow/core/ShadowApplication.h"
#include "runtime/runtime.h"

#include "EditorModule.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>

int main(int argc, char *argv[]) {
    RUNTIME_BOOT();

    std::cout << "CWD: " << std::filesystem::current_path().c_str() << '\n';

    std::cout << "argc == " << argc << '\n';

    for (int ndx{}; ndx != argc; ++ndx) {
        std::cout << "argv[" << ndx << "] == " << std::quoted(argv[ndx]) << '\n';
    }
    std::cout << "argv[" << argc << "] == " << static_cast<void *>(argv[argc]) << '\n';
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
                         .dependencies={"module:/platform/sdl2"},
                     });
}
#pragma clang diagnostic pop
}


