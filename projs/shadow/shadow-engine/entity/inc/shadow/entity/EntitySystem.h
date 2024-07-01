#pragma once

#include "shadow/core/Module.h"

#include "graph/graph.h"
#include "NodeManager.h"
#include "shadow/event-bus/events.h"

//Holds the reference to the active scene

namespace SH::Entities {

    /**
     * The module that manages all the entities and Scenes
     */
    class API EntitySystem : public SH::Module {
      SHObject_Base(EntitySystem)
      private:
        World world;

      public:
        EntitySystem();

        ~EntitySystem() override;

        World &GetWorld() { return world; }

        // event functions

        void Init() override;

        void Update(int frame) override;
    };

}