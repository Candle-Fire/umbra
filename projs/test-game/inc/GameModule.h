#pragma once

#include "shadow/core/convar.h"
#include "shadow/core/Module.h"

extern ConVar var_test;

class GameModule : public SH::Module {

SHObject_Base(GameModule)

  std::string test = "asdasd";
public:
  GameModule() : Module() {}

  void PreInit() override;

  void Init() override;
};