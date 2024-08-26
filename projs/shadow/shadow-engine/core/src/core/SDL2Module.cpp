#include "shadow/core/SDL2Module.h"
#include "shadow/SHObject.h"
#include "shadow/core/ShadowWindow.h"
#include "shadow/core/module-manager-v2.h"
#include "spdlog/spdlog.h"

#include "shadow/core/ShadowApplication.h"
#include <SDL.h>
#include <SDL_events.h>

SHObject_Base_Impl(SH::SDL2Module)

MODULE_ENTRY(SH::SDL2Module, SDL2Module)

void SH::SDL2Module::PreInit() {
    // Initialize SDL. SDL_Init will return -1 if it fails.
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        spdlog::error("Error creating window: " + std::string(SDL_GetError()));
        //system("pause");
        // End the program
        //return 1;
    }

    window = new ShadowWindow(1280, 720);

    //SDL_SetRelativeMouseMode(SDL_TRUE);
}

void SH::SDL2Module::Update(int frame)
{
    static SDL_Event event;
    while (SDL_PollEvent(&event)) {  // poll until all events are handled!
        if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED)
        {
            window->UpdateSize();
        }


        Events::SDLEvent e(event);
        Events::EventDispatcher<Events::SDLEvent>::call(e);

        if (event.type == SDL_QUIT)
        {
            ShadowApplication::Get().Stop();
        }
    }
}

void SH::SDL2Module::Init() {
    // ShadowApplication::Get().GetEventBus().subscribe<Events::SDLEvent>([this](auto &&PH1) {
    //     SDLEvent(std::forward<decltype(PH1)>(PH1));
    // });
}

void SH::SDL2Module::Destroy() {
    SDL_DestroyWindow(window->sdlWindowPtr);
    SDL_Quit();
}