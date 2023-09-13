#pragma once

#include <SDL_events.h>
#include "shadow/core/Module.h"

class GameModule : public SH::Module {

SHObject_Base(GameModule)

  std::string test = "asdasd";
public:
  GameModule() : Module() {}

  void PreInit() override;

  void Init() override;
};