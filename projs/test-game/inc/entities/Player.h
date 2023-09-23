#pragma once

#include "shadow/entitiy/graph/graph.h"

class [[SH::Reflect]] Player : public SH::Entities::Actor {
SHObject_Base(Player);
SHReflect(Player);

public:
  Player(std::string name) : Actor() {
      this->name = name;
  }

  void Build() override;
};
