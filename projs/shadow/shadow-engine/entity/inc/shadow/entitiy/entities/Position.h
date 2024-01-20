#pragma once

#include "shadow/entitiy/graph/graph.h"

//Position component in the builtin namespace
namespace SH::Entities::Builtin {

    class API Position : public SH::Entities::Component {
      SHObject_Base(Position)
      public:
        float x = 0;
        float y = 0;
        float z = 0;

        Position() = default;

        Position(float x_, float y_, float z_)
            : x(x_), y(y_), z(z_) {}
    };
}
