#pragma once

#include "Module.h"
#include "ShadowWindow.h"

#include "SDL.h"

#include "event-bus/events.h"

namespace ShadowEngine {

    class SDL2Module : public Module {
      SHObject_Base(SDL2Module)

      public:
        ShadowEngine::ShadowWindow *window;

        SDL2Module() : Module() {}

      private:
        void SDLEvent(SH::Events::SDLEvent &sdl_event);

        void Init() override;

        void PreInit() override;

        void Update(int frame) override {};

        void Recreate() override {};

        void Render(VkCommandBuffer &commands, int frame) override {};

        void OverlayRender() override {};

        void LateRender(VkCommandBuffer &commands, int frame) override {};

        void AfterFrameEnd() override {};

        void PreRender() override {};

        void Destroy() override;
    };

}