#pragma once

#include "graph/graph.h"

class TestScene : public ShadowEngine::Entities::Scene {
  SHObject_Base(TestScene)
  public:
    TestScene() : Scene("Test scene") {}

    void Build() override;
};
