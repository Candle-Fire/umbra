#pragma once

#include <shadow/event-bus/events.h>
#include "shadow/SHObject.h"

#include <SDL.h>


namespace SH
{
  class ShadowWindow {
  public:

    int Height;
    int Width;

    SDL_Window *sdlWindowPtr;

    SDL_Surface *sdlSurface = NULL;


    //ShadowEngine::Ref<ShadowEngine::Rendering::GraphicsContext> context;

    ShadowWindow(int W, int H);

    ~ShadowWindow();

    void UpdateSize();

  };

  class WindowResizeEvent final : public Events::Event
  {
    SHObject_Base(WindowResizeEvent)

  public:
    int Width;
    int Height;

    WindowResizeEvent(const int width, const int height)
      : Width(width),
        Height(height)
    {
    }
  };
}