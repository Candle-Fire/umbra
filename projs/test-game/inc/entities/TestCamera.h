#pragma once

#include "shadow/entity/graph/graph.h"

//Example of a 2d camera component with size
class TestCamera : public SH::Entities::Component {
  SHObject_Base(TestCamera);
  public:
    float width;
    float height;
};
