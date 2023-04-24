#pragma once

#include "graph/graph.h"

//Example of a 2d camera component with size
class [[SH::Reflect]] TestCamera : public ShadowEngine::Entities::Component {
  SHObject_Base(TestCamera);
  public:
    [[SH::Reflect]] float width;
    [[SH::Reflect]] float height;
};
