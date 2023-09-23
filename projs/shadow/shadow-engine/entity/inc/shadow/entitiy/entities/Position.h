#pragma once

#include "shadow/entitiy/graph/graph.h"

namespace SH::Entities::Builtin {

  class API [[SH::Reflect]] Position : public SH::Entities::Component {
  SHObject_Base(Position)
  SHReflect(Player);
  public:
    [[SH::Reflect]] float x = 0;
    float y = 0;
    float z = 0;

    Position() = default;

    Position(float x_, float y_, float z_)
        : x(x_), y(y_), z(z_) {}
  };
}
