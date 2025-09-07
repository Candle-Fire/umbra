#pragma once
#include "components.h"
#include "modules/ecs/ecs.h"

class Player final : public Entity
{
public:
  Player() = default;
  ~Player() override = default;

  void Init() override;
};
