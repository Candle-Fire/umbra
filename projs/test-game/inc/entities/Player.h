#pragma once

#include "graph/graph.h"

class Player : public ShadowEngine::Entities::Actor {
  SHObject_Base(Player);

  public:
    Player(std::string name) : Actor() {
        this->name = name;
    }

    void Build() override;
};
