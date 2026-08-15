#pragma once
#include "SDL3/SDL_video.h"
#include "shadow/core/ShadowWindow.h"

namespace SH
{
  class SDL3Window : Window
  {
  public:
    SDL_Window *sdlWindowPtr;

    SDL_Surface *sdlSurface = NULL;

    SDL3Window(int W, int H);
  };
}
