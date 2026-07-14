#define STB_IMAGE_IMPLEMENTATION

#include <spdlog/spdlog.h>

#include "runtime/Runtime.h"
#include "shadow/core/ShadowApplication.h"
#include "shadow/core/Time.h"
#include "shadow/renderer/vulkan/vlkx/vulkan/abstraction/Commands.h"
#include "shadow/renderer/vulkan/vlkx/vulkan/VulkanModule.h"

#include "shadow/core/convar.h"
#include "shadow/log/LoggerModule.h"
#include "shadow/platform/console-setup.h"
#include "shadow/assets/fs/file.h"
#include "shadow/jobs/Job.h"

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

namespace SH {

  dylib *gameLib;

  SHObject_Base_Impl(ShadowApplication)

  ShadowApplication *ShadowApplication::instance = nullptr;

  std::unique_ptr<FileSystem> ShadowApplication::diskFS = FileSystem::createDiskFS(Path("./"));

  std::unique_ptr<vlkx::RenderCommand> renderCommands;

  std::weak_ptr<VulkanModule> renderer;

  ShadowApplication::ShadowApplication(int argc, char *argv[]) {
      instance = this;

      if (argc > 1) {
          for (size_t i = 0; i < argc; i++) {
              std::string param(argv[i]);
              if (param == "-trace") {
                  this->debug = true;
              }
              if (param == "-game") {
                  this->game = argv[i + 1];
              }
          }
      }

      auto cvarmgr = ConVarManager::Get();
      cvarmgr->ParseArgs(argc, argv);

      InitBasicLogger();

      InitConsole();
  }

  ShadowApplication::~ShadowApplication() {
  }

  void ShadowApplication::Init() {
      Runtime::Runtime::Get().AddAssembly({"assembly:/core"_id, "shadow-engine"});
      moduleManager.LoadModulesFromAssembly("assembly:/core"_id);

      SH::Jobs::init();

      if (!game.empty()) {
          spdlog::info("Loading Game: {0}", game);
          const auto &id = SH::Path("assembly:/" + game);
          Runtime::Runtime::Get().AddAssembly({id, "./" + game});
          moduleManager.LoadModulesFromAssembly(id);
      }

      moduleManager.Init();

      resourceManager.init(*diskFS);

      renderer = moduleManager.GetById<VulkanModule>("module:/renderer/vulkan");

      renderCommands = std::make_unique<vlkx::RenderCommand>(2);
  }

  void ShadowApplication::Start() {
      SDL_Event event;
      while (running) {
          while (SDL_PollEvent(&event)) {  // poll until all events are handled!
              eventBus.fire(Events::SDLEvent(event));
              if (event.type == SDL_EVENT_QUIT)
                  running = false;
          }

          //eventBus.fire(SH::Events::PreRender());

          if (!renderer.expired()) {
              auto r = renderer.lock();
              r->BeginRenderPass(renderCommands);
          }

          renderCommands->nextFrame();
          SH::Timer::UpdateTime();
      }

      //moduleManager.Destroy();

      delete gameLib;
  }

  ShadowApplication &ShadowApplication::Get() { return *instance; };
}
