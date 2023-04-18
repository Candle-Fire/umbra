#pragma once

#include "imgui.h"
#include "EntitySystem.h"
#include "core/module-manager-v2.h"
#include "core/ShadowApplication.h"

namespace ShadowEngine::Entities::Debugger {

    class AllocationDebugger {

        static std::weak_ptr<ShadowEngine::Entities::EntitySystem> entitySystem;

      public:
        static void Draw() {

            if (entitySystem.expired())
                entitySystem =
                    ShadowEngine::ShadowApplication::Get().GetModuleManager().GetById<ShadowEngine::Entities::EntitySystem>(
                        "module:/entity-system");

            DrawAllocationDebugger();

        }

        static void DrawAllocationDebugger();
    };

}
