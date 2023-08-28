#pragma once

#include <SDL2/SDL.h>

#include "Module.h"
#include "ShadowWindow.h"
#include "shadow/event-bus/events.h"

namespace SH {

    class SDL2Module : public Module {
      SHObject_Base(SDL2Module)

      public:
        SH::ShadowWindow *window;

        SDL2Module() : Module() {}

      private:
        void SDLEvent(SH::Events::SDLEvent &sdl_event);

        void Init() override;

        void PreInit() override;

        void Destroy() override;
    };

}