#include "core/ShadowWindow.h"


ShadowWindow::ShadowWindow(int W, int H) : Height(H), Width(W)
{
    // Create our window
    sdlWindowPtr = SDL_CreateWindow( "Candlefire", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Width, Height, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN );

    // Make sure creating the window succeeded
    if ( !sdlWindowPtr ) {
        //Raise an error in the log
        //std::cout << "Error creating window: " << SDL_GetError()  << std::endl;
    }

}

ShadowWindow::~ShadowWindow()
{
}
