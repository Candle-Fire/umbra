#pragma once

#include "SDL.h"

namespace SH {

  class ShadowWindow {
  public:
    int Height;
    int Width;

    SDL_Window *sdlWindowPtr;

    SDL_Surface *sdlSurface = NULL;

    ShadowWindow(int W, int H);

    ~ShadowWindow();
  };

}