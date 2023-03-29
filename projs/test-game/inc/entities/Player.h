#pragma once

#include "graph/graph.h"

class Player : public ShadowEngine::Entities::Actor{
    SHObject_Base(Player);

public:
    void Build() override;
};
