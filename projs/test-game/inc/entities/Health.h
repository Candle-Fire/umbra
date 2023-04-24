#pragma once

#include "graph/graph.h"

class [[SH::Reflect]] Health : public ShadowEngine::Entities::Component {
  SHObject_Base(Health);
  public:
    [[SH::Reflect]] int health;
};
