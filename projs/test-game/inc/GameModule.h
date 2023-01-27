#pragma once

#include <SDL_events.h>
#include "core/Module.h"

class GameModule : public ShadowEngine::Module {

    SHObject_Base(GameModule)

    std::string test = "asdasd";
public:
    GameModule() : Module("module:/test-game/GameModule") {}

    void PreInit() override;

    void Init() override;

    void Update(int frame) override;

    void PreRender() override;

    void Recreate() override;

    void OverlayRender() override;

    void Render(VkCommandBuffer& commands, int frame) override;

    void LateRender(VkCommandBuffer& commands, int frame) override;

    void AfterFrameEnd() override;

    void Destroy() override;

    void Event(SDL_Event*) override;

};