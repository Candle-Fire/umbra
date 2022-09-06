#pragma once

#include <SDL_events.h>
#include "core/Module.h"

class GameModule : public ShadowEngine::Module {

    SHObject_Base(GameModule)

    std::string tets = "asdasd";
public:
    void PreInit() override;

    void Init() override;

    void Update() override;

    void PreRender() override;

    void Render() override;

    void LateRender() override;

    void AfterFrameEnd() override;

    void Destroy() override;

    void Event(SDL_Event*) override;

};