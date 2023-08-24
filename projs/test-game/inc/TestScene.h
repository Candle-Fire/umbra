#pragma once

#include "graph/graph.h"
#include "event-bus/events.h"

class TestScene : public ShadowEngine::Entities::Scene {
  SHObject_Base(TestScene)
  public:
    TestScene() : Scene("Test scene") {}

    void Build() override;
    void Recreate(SH::Events::Recreate&);
};
