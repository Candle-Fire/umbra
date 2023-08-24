#include "core/SDL2Module.h"
#include "core/ShadowWindow.h"
#include "core/module-manager-v2.h"
#include "spdlog/spdlog.h"
#include "imgui_impl_sdl2.h"

#include "core/ShadowApplication.h"

SHObject_Base_Impl(ShadowEngine::SDL2Module)

MODULE_ENTRY(ShadowEngine::SDL2Module, SDL2Module)

void ShadowEngine::SDL2Module::PreInit() {
    // Initialize SDL. SDL_Init will return -1 if it fails.
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        spdlog::error("Error creating window: " + std::string(SDL_GetError()));
        //system("pause");
        // End the program
        //return 1;
    }

    window = new ShadowWindow(1280, 720);
    SDL_SetWindowResizable(window->sdlWindowPtr, SDL_TRUE);
    //SDL_SetRelativeMouseMode(SDL_TRUE);
}

void ShadowEngine::SDL2Module::Init() {
    ShadowEngine::ShadowApplication::Get().GetEventBus().subscribe<SH::Events::SDLEvent>([this](auto &&PH1) {
        SDLEvent(std::forward<decltype(PH1)>(PH1));
    });
}

void ShadowEngine::SDL2Module::SDLEvent(SH::Events::SDLEvent &sdl_event) {
    ImGui_ImplSDL2_ProcessEvent(&sdl_event.event);
}

void ShadowEngine::SDL2Module::Destroy() {
    SDL_DestroyWindow(window->sdlWindowPtr);
    SDL_Quit();
}