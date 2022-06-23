#include "ShadowApplication.h"

#include "Time.h"

#include <string>

namespace ShadowEngine {

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
			}
		}
			
		
		//game = _setupFunc();
	}


	ShadowApplication::~ShadowApplication()
	{
	}

	void ShadowApplication::Init()
	{
        // Initialize SDL. SDL_Init will return -1 if it fails.
        if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
            //std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
            //system("pause");
            // End the program
            //return 1;
        }

        window_ = new ShadowWindow(800,450);

        /*
		moduleManager.PushModule(new Log());
		moduleManager.PushModule(new EventSystem::ShadowEventManager());
		moduleManager.PushModule(new SDLPlatform::SDLModule());
		moduleManager.PushModule(new Rendering::Renderer());

		moduleManager.PushModule(new Assets::AssetManager());

		if(!no_gui)
		moduleManager.PushModule(new DebugGui::ImGuiModule());
		
		moduleManager.PushModule(new InputSystem::ShadowActionSystem());
		//moduleManager.PushModule(new Debug::DebugModule());
		moduleManager.PushModule(new EntitySystem::EntitySystem());


		game->Init();
		
		moduleManager.Init();
        */
	}

	void ShadowApplication::Start()
	{
		while (running)
		{
			Time::UpdateTime();
		}
	}
}
