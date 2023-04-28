#pragma once

#include "graph/graph.h"

//Position component in the builtin namespace
namespace ShadowEngine::Entities::Builtin {

    class [[SH::Reflect, API_NEW]] Position : public ShadowEngine::Entities::Component {
      SHReflect(Position);
      SHObject_Base(Position)
      public:
        [[SH::Reflect]]float x = 0;
        [[SH::Reflect]]float y = 0;
        [[SH::Reflect]]float z = 0;

        Position() = default;

        Position(float x_, float y_, float z_)
            : x(x_), y(y_), z(z_) {}
    };
}
