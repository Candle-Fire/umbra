#pragma once

#include "shadow/entity/graph/graph.h"

class Player : public SH::Entities::Actor {
  SHObject_Base(Player);

  public:
    Player(std::string name) : Actor() {
        this->name = name;
    }

    void Build() override;
};
