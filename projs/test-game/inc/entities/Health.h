#pragma once

#include "shadow/entity/graph//graph.h"

class Health : public SH::Entities::Component {
  SHObject_Base(Health);
  public:
    int health;
};
