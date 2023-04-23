#pragma once

#include <SDL_events.h>
#include "core/Module.h"

class GameModule : public ShadowEngine::Module {

  SHObject_Base(GameModule)

    std::string test = "asdasd";
  public:
    GameModule() : Module() {}

    void PreInit() override;

    void Init() override;
};