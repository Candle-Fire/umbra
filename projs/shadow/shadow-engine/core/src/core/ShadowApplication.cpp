#define STB_IMAGE_IMPLEMENTATION

#include "core/ShadowApplication.h"
#include "core/Time.h"
#include "dylib.hpp"
#include "vlkx/vulkan/abstraction/Commands.h"
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <spdlog/spdlog.h>

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

namespace ShadowEngine {

    dylib* gameLib;

	ShadowApplication* ShadowApplication::instance = nullptr;

    std::unique_ptr<vlkx::RenderCommand> renderCommands;

    ShadowApplication::ShadowApplication(int argc, char* argv[])
	{
		instance = this;

		if(argc > 1)
		{
			for (size_t i = 0; i < argc; i++)
			{
				std::string param(argv[i]);
				if(param == "-no-gui")
				{
					this->no_gui = true;
				}
                if(param == "-game")
                {
                    this->game = argv[i+1];
                }
			}
		}
        SetConsoleOutputCP(CP_UTF8);
        CONSOLE_FONT_INFOEX cfi;
        cfi.cbSize = sizeof cfi;
        cfi.nFont = 0;
        cfi.dwFontSize.X = 0;
        cfi.dwFontSize.Y = 14;
        cfi.FontFamily = FF_DONTCARE;
        cfi.FontWeight = FW_NORMAL;
        wcscpy_s(cfi.FaceName, LF_FACESIZE, L"Lucida Console");
        if (SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi) == 0) {
            // handle error
        }

        spdlog::set_level(spdlog::level::debug);
	}

	ShadowApplication::~ShadowApplication()
	{
	}

    void ShadowApplication::loadGame(){
        if(game.empty())
            return;

        void (*gameInti)(ShadowApplication*);

        try {
            gameLib = new dylib("./", game);

            gameInti = gameLib->get_function<void(ShadowApplication*)>("shadow_main");

            gameInti(this);
        }
        catch (std::exception& e) {
            spdlog::error(e.what());
            exit(1);
        }

    }

	void ShadowApplication::Init()
	{
        moduleManager.AddDescriptors({
                                             .id="module:/renderer/vulkan",
                                             .name = "Vulkan",
                                             .class_name = "VulkanModule",
                                             .assembly="shadow-engine",
                                             .dependencies={"module:/platform/sdl2"},
                                     });

        moduleManager.AddDescriptors({
            .id="module:/core",
            .name = "Core",
                                             .class_name = "",
            .assembly="shadow-engine",
            });

        moduleManager.AddDescriptors({
                                             .id="module:/platform/sdl2",
                                             .name = "SDL2",
                                             .class_name = "SDL2Module",
                                             .assembly="shadow-engine",
                                             .dependencies={"module:/core"},
                                     });



        moduleManager.AddDescriptors({
                                             .id="module:/test1",
                                             .name = "Test1",
                                             .class_name = "",
                                             .assembly="shadow-engine",
                                     });

        //moduleManager.PushModule(std::make_shared<SDL2Module>(),"core");
        //auto renderer = std::make_shared<VulkanModule>();
        //renderer->EnableEditor();
        //moduleManager.PushModule(renderer, "renderer");

        //moduleManager.PushModule(std::make_shared<Debug::DebugModule>(), "core");

        //loadGame();



        moduleManager.Init();
        //renderCommands = std::make_unique<vlkx::RenderCommand>(2);
	}

	void ShadowApplication::Start()
	{
        SDL_Event event;
		while (running)
		{
            while (SDL_PollEvent(&event)) {  // poll until all events are handled!
                //moduleManager.Event(&event);
                if (event.type == SDL_QUIT)
                    running = false;
            }

            //moduleManager.PreRender();

            //moduleManager.renderer->BeginRenderPass(renderCommands);

            //moduleManager.AfterFrameEnd();

            //renderCommands->nextFrame();
            Time::UpdateTime();
		}

        //moduleManager.Destroy();

        delete gameLib;
	}

    ShadowApplication& ShadowApplication::Get() { return *instance; };
}
