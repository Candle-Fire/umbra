#include "core/ShadowApplication.h"

#include "core/Time.h"
#include "core/SDL2Module.h"
#include "debug/DebugModule.h"
#include "dylib.hpp"


#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_sdl.h>
#include <vlkx/vulkan/VulkanManager.h>
#include <vlkx/render/Camera.h>
#include <vlkx/render/geometry/SingleRenderer.h>
#include <spdlog/spdlog.h>

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

namespace ShadowEngine {

    dylib* gameLib;

	ShadowApplication* ShadowApplication::instance = nullptr;

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
        loadGame();

        printf("exe side: %p \n", VulkanManager::getInstance());
        printf("exe next ID: %llu \n", ShadowEngine::SHObject::GenerateId());

        moduleManager.PushModule(std::make_shared<SDL2Module>(),"core");
        moduleManager.PushModule(std::make_shared<Debug::DebugModule>(), "core");

        moduleManager.Init();

        auto sdl2module = moduleManager.GetModuleByType<SDL2Module>();

        window_ = sdl2module->_window;


	}

	void ShadowApplication::Start()
	{
        SDL_Event event;
		while (running)
		{
            while (SDL_PollEvent(&event)) {  // poll until all events are handled!
                moduleManager.Event(&event);
                if (event.type == SDL_QUIT)
                    running = false;
            }

            moduleManager.Update();
            moduleManager.PreRender();

            VulkanManager::getInstance()->startDraw();
            moduleManager.Render();

            moduleManager.LateRender();
            VulkanManager::getInstance()->endDraw();

            moduleManager.AfterFrameEnd();
		}

        moduleManager.Destroy();

        delete gameLib;
	}

    ShadowApplication& ShadowApplication::Get() { return *instance; };
}
