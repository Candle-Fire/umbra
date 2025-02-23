#include "shadow/core/ShadowWindow.h"
#include <spdlog/spdlog.h>

#include <string>

SH::ShadowWindow::ShadowWindow(int W, int H) : Height(H), Width(W)
{
    // Start our window
    sdlWindowPtr = SDL_CreateWindow( "Candlefire", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Width, Height, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN );

    // Make sure creating the window succeeded
    if ( !sdlWindowPtr ) {
        //Raise an error in the log
        spdlog::error("Error creating window: " + std::string(SDL_GetError()));
    }

}

SH::ShadowWindow::~ShadowWindow()
{
}
