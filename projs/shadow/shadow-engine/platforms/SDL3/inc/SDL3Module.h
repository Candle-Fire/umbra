#pragma once

#include <memory>
#include <SDL3/SDL.h>

#include "SDL3Window.h"
#include "../../../core/inc/shadow/core/Module.h"
#include "shadow/event-bus/events.h"

namespace SH {

  class SDL3Module : public Module {
  SHObject_Base(SDL3Module)

  public:
    std::unique_ptr<SDL3Window> window;

    SDL3Module() : Module() {}

  private:
    void SDLEvent(Events::SDLEvent &sdl_event);

    void Init() override;

    void PreInit() override;

    void Destroy() override;
  };

}