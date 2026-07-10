#include "../inc/SDL3Module.h"
#include "shadow/core/ShadowWindow.h"
#include "shadow/core/module-manager-v2.h"
#include "spdlog/spdlog.h"
//#include "imgui_impl_sdl2.h"

#include "shadow/core/ShadowApplication.h"

SHObject_Base_Impl(SH::SDL3Module)

MODULE_ENTRY(SH::SDL3Module, SDL3Module)

void SH::SDL3Module::PreInit() {
    // Initialize SDL. SDL_Init will return -1 if it fails.
    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO)) {
        spdlog::error("Error creating window: " + std::string(SDL_GetError()));
    }

    window = std::make_unique<SDL3Window>(1280, 720);
  spdlog::trace("Created window");
    SDL_SetWindowResizable(window->sdlWindowPtr, true);
}

void SH::SDL3Module::Init() {
    ShadowApplication::Get().GetEventBus().subscribe<SH::Events::SDLEvent>([this](auto &&PH1) {
        SDLEvent(std::forward<decltype(PH1)>(PH1));
    });
}

void SH::SDL3Module::SDLEvent(SH::Events::SDLEvent &sdl_event) {
}

void SH::SDL3Module::Destroy() {
    SDL_DestroyWindow(window->sdlWindowPtr);
    SDL_Quit();
}