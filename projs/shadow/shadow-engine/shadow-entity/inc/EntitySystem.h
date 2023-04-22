#pragma once

#include "core/Module.h"

#include "graph/graph.h"
#include "NodeManager.h"

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

        void PreInit() override {};

        void Init() override;

        void Update(int frame) override;

        void Render(VkCommandBuffer &commands, int frame) override {};

        void LateRender(VkCommandBuffer &commands, int frame) override {};

        void Recreate() override {}

        void PreRender() override {}

        void OverlayRender() override;

        void AfterFrameEnd() override {}

        void Destroy() override {}
    };

}