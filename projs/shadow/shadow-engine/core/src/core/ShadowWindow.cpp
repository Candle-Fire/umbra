#include "shadow/core/ShadowWindow.h"
#include <spdlog/spdlog.h>

#include <string>
#include <shadow/event-bus/event_bus.h>

SH::ShadowWindow::ShadowWindow(const int W, const int H) : Height(H), Width(W)
{
    // Create our window
    sdlWindowPtr = SDL_CreateWindow( "Candle-fire", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Width, Height, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    // Make sure creating the window succeeded
    if ( !sdlWindowPtr ) {
        //Raise an error in the log
        spdlog::error("Error creating window: " + std::string(SDL_GetError()));
    }

    SDL_SetWindowMaximumSize(sdlWindowPtr, 1920, 1080);

}

SH::ShadowWindow::~ShadowWindow()
{
    SDL_DestroyWindow(sdlWindowPtr);
}

void SH::ShadowWindow::UpdateSize()
{
    SDL_GetWindowSize(sdlWindowPtr, &Width, &Height);

    spdlog::info("Window resized to: " + std::to_string(Width) + "x" + std::to_string(Height));

    WindowResizeEvent e(Width, Height);
    Events::EventDispatcher<WindowResizeEvent>::call(e);
}

namespace SH::Events
{
    Event_Impl(SH::WindowResizeEvent)
}

SHObject_Base_Impl(SH::WindowResizeEvent)

