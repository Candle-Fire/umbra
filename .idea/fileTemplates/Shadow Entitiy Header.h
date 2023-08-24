#pragma once

#include "Entity.h"
#include "TransformEntity.h"

class ${NAME} : public ShadowEngine::Entities::TransformEntity{
    SHObject_Base(${NAME});

    Entity_Base(${NAME}, TransformEntity);

public:
    void Build() override;
};
