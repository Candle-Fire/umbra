#pragma once

#include "shadow/entity/graph/graph.h"

class TestScene : public SH::Entities::Scene {
  SHObject_Base(TestScene)
  public:
    TestScene() : Scene("Test scene") {}

    void Build() override;
};
