#pragma once


#include "Entity.h"
#include "TransformEntity.h"
#include "TestCamera.h"

class Player : public ShadowEngine::Entities::TransformEntity{
    SHObject_Base(Player);

    Entity_Base(Player, TransformEntity);

public:

    void Build() override;

};
