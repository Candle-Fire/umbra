#include "core/ShadowApplication.h"
#include "../inc/EditorModule.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>



int main(int argc, char *argv[])
{
    std::cout << "argc == " << argc << '\n';

    for(int ndx{}; ndx != argc; ++ndx) {
        std::cout << "argv[" << ndx << "] == " << std::quoted(argv[ndx]) << '\n';
    }
    std::cout << "argv[" << argc << "] == " << static_cast<void*>(argv[argc]) << '\n';
    /*...*/

    ShadowEngine::ShadowApplication app(argc, argv);
    app.Init([](ShadowEngine::ModuleManager& mgr){
        mgr.PushModule(std::make_shared<ShadowEngine::Editor::EditorModule>(),"core");
    });
    app.Start();


    return argc == 3 ? EXIT_SUCCESS : EXIT_FAILURE; // optional return value
}