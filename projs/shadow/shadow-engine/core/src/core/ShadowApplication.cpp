#define STB_IMAGE_IMPLEMENTATION

#include <spdlog/spdlog.h>

#include "runtime/Runtime.h"
#include "shadow/core/ShadowApplication.h"
#include "shadow/core/Time.h"

#include "shadow/platform/console-setup.h"
#include "shadow/assets/fs/file.h"

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

namespace SH {

  dylib *gameLib;

  SHObject_Base_Impl(ShadowApplication)

  ShadowApplication *ShadowApplication::instance = nullptr;

  std::unique_ptr<ShadowEngine::FileSystem> ShadowApplication::diskFS = ShadowEngine::FileSystem::createDiskFS("./");

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

      spdlog::set_level(spdlog::level::trace);

      InitConsole();
  }

  ShadowApplication::~ShadowApplication() {
  }

  void ShadowApplication::Init() {
      Runtime::Runtime::Get().AddAssembly({"assembly:/core"_id, "shadow-engine"});
      moduleManager.LoadModulesFromAssembly("assembly:/core"_id);

      if (!game.empty()) {
          spdlog::info("Loading Game: {0}", game);
          const auto &id = SH::Path("assembly:/" + game);
          Runtime::Runtime::Get().AddAssembly({id, "./" + game});
          moduleManager.LoadModulesFromAssembly(id);
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
          Timer::UpdateTime();
      }

      //moduleManager.Destroy();

      delete gameLib;
  }

  ShadowApplication &ShadowApplication::Get() { return *instance; };
}
