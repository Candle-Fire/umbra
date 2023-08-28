#pragma once

#include "shadow/entitiy/graph//graph.h"

class Health : public SH::Entities::Component {
  SHObject_Base(Health);
  public:
    int health;
};
