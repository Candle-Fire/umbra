#include "../inc/SDL3Window.h"

#include <string>

#include "spdlog/spdlog.h"

namespace SH
{
  SDL3Window::SDL3Window(const int W, const int H): Window(W, H)
  {

    // Create our window
    sdlWindowPtr = SDL_CreateWindow( "Candlefire", Width, Height, SDL_WINDOW_VULKAN );

    // Make sure creating the window succeeded
    if ( !sdlWindowPtr ) {
      //Raise an error in the log
      spdlog::error("Error creating window: " + std::string(SDL_GetError()));
    }

  }
}
