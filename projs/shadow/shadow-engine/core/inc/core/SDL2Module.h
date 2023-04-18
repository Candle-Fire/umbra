#pragma once

#include "Module.h"
#include "ShadowWindow.h"

#include "SDL.h"

namespace ShadowEngine {

    class SDL2Module : public Module {
      SHObject_Base(SDL2Module)

      public:
        ShadowEngine::ShadowWindow *window;

      private:
      public:
        SDL2Module() : Module() {}

      private:

        void Init() override;

        void PreInit() override;

        void Update(int frame) override;

        void Recreate() override;

        void Render(VkCommandBuffer &commands, int frame) override;

        void OverlayRender() override;

        void LateRender(VkCommandBuffer &commands, int frame) override;

        std::string GetName() override;

        void AfterFrameEnd() override;

        void PreRender() override;

        void Destroy() override;

        void Event(SDL_Event *e) override;
    };

}