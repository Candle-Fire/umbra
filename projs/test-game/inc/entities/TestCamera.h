#pragma once

#include "shadow/entitiy/graph/graph.h"

namespace test_game {
    
    class [[SH::Reflect]] TestCamera : public ShadowEngine::Entities::Component {
      SHReflect(TestCamera);
      SHObject_Base(TestCamera);
      public:
        [[SH::Reflect]] float width;
        [[SH::Reflect]] float height;
    };

}