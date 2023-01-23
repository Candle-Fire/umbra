#pragma once

#include "Entity.h"

namespace ShadowEngine::Entities::Builtin {

    class NullEntity : public ShadowEngine::Entities::Entity{
        SHObject_Base(NullEntity);

        Entity_Base_NoCtor(NullEntity, Entity);

    public:
        NullEntity() {
            this->name = "Null entity";
        }

        explicit NullEntity(Scene *scene) : Entity(scene) {
            this->name = "Null entity";
        }

        ~NullEntity() override {

        }
    };

} // Builtin
