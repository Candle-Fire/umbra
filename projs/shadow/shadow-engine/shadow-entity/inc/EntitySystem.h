#pragma once

#include "core/Module.h"

#include "graph/graph.h"
#include "NodeManager.h"
#include "event-bus/events.h"
#include "event-bus/render_events.h"

//Holds the reference to the active scene

namespace ShadowEngine::Entities {

    /**
     * The module that manages all the entities and Scenes
     */
    class API EntitySystem : public ShadowEngine::Module {
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

        void OverlayRender(SH::Events::OverlayRender &overlay);
        void Render(SH::Events::Render &render);
    };

}