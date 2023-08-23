#define STB_IMAGE_IMPLEMENTATION

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <spdlog/spdlog.h>

#include "core/ShadowApplication.h"
#include "core/Time.h"
#include "dylib.hpp"
#include "vlkx/vulkan/abstraction/Commands.h"
#include "vlkx/vulkan/VulkanModule.h"
#include "../../../../shadow-editor/inc/EditorWindow.h"
#include "renderer/RenderOrchestrator.h"

#include <d3d12.h>
#include <directxmath.h>

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

namespace ShadowEngine {

    dylib *gameLib;

    SHObject_Base_Impl(ShadowApplication)

    ShadowApplication *ShadowApplication::instance = nullptr;

    ShadowApplication::ShadowApplication(int argc, char *argv[]) {
        instance = this;

        if (argc > 1) {
            for (size_t i = 0; i < argc; i++) {
                std::string param(argv[i]);
                if (param == "-debug") {
                    this->debug = true;
                }
                if (param == "-game") {
                    this->game = argv[i + 1];
                }
            }
        }

        if (this->debug)
            spdlog::set_level(spdlog::level::debug);
    }

    ShadowApplication::~ShadowApplication() {
    }

    void ShadowApplication::Init() {
        moduleManager.AddAssembly({.id="assembly:/core", .path="shadow-engine"});
        moduleManager.LoadModulesFromAssembly("assembly:/core");

        if (!game.empty()) {
            spdlog::info("Loading Game: {0}", game);
            moduleManager.AddAssembly({.id="assembly:/" + game, .path=game});
            moduleManager.LoadModulesFromAssembly("assembly:/" + game);
        }

        moduleManager.Init();
    }

    void ShadowApplication::Start() {
        SDL_Event event;
        while (running) {
            while (SDL_PollEvent(&event)) {  // poll until all events are handled!
                SH::Events::SDLEvent e(event);
                SH::Events::EventDispatcher<SH::Events::SDLEvent>::call(e);
                //eventBus.fire(e);
                if (event.type == SDL_QUIT)
                    running = false;
            }

            eventBus.fire(SH::Events::PreRender());
            eventBus.fire(SH::Events::Render());

            auto submitter = moduleManager.GetById<vlkx::RenderOrchestrator>("module:/render").lock()->getSubmitter();

            if (!submitter.expired()) submitter.lock()->Draw();

            Time::UpdateTime();
        }

        //moduleManager.Destroy();

        delete gameLib;
    }

    ShadowApplication &ShadowApplication::Get() { return *instance; };
}
