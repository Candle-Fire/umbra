#pragma once

#include "graph/graph.h"

namespace test_game {

    class [[SH::Reflect]] Health : public ShadowEngine::Entities::Component {
      SHReflect(TestCamera);
      SHObject_Base(Health);
      public:
        [[SH::Reflect]] int health;
    };

}