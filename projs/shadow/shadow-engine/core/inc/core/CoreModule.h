#pragma once

#include "Module.h"

class CoreModule : public ShadowEngine::Module {
  SHObject_Base(CoreModule)
  public:
    CoreModule() : Module() {}

    void PreInit() override {

    }

    void Init() override {

    }

    void Update(int frame) override {

    }

    void Recreate() override {

    }

    void PreRender() override {

    }

    void Render(VkCommandBuffer &commands, int frame) override {

    }

    void LateRender(VkCommandBuffer &commands, int frame) override {

    }

    void OverlayRender() override {

    }

    void AfterFrameEnd() override {

    }

    void Destroy() override {

    }

    void Event(SDL_Event *e) override {

    }
};
