#pragma once

#include "graph/graph.h"

class [[SH::Reflect]] Player : public ShadowEngine::Entities::Actor {
  SHObject_Base(Player);
  public:
    [[SH::Reflect]] int hp;

  public:
    Player(std::string name) : Actor() {
        this->name = name;
    }

    void Build() override;
};
