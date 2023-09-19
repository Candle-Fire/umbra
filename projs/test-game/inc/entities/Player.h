#pragma once

#include "shadow/entitiy/graph/graph.h"

namespace test_game {

    class [[SH::Reflect]] Player : public ShadowEngine::Entities::Actor {
      SHReflect(Player);
      SHObject_Base(Player);
      public:
        [[SH::Reflect]] int hp = 10;

      public:
        Player(std::string name) : Actor() {
            this->name = name;
        }

        void Build() override;
    };

}