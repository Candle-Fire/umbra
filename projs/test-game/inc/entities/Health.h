#pragma once

#include "graph/graph.h"

class Health : public ShadowEngine::Entities::Component {
  SHObject_Base(Health);
  public:
    int health;
};
