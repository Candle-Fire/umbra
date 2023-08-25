#pragma once

#include "imgui.h"
#include "EntitySystem.h"
#include "core/module-manager-v2.h"
#include "core/ShadowApplication.h"

namespace SH::Entities::Debugger {

    class AllocationDebugger {

        static std::weak_ptr<SH::Entities::EntitySystem> entitySystem;

      public:
        static void Draw() {

            if (entitySystem.expired())
                entitySystem =
                    SH::ShadowApplication::Get().GetModuleManager().GetById<SH::Entities::EntitySystem>(
                        "module:/entity-system");

            DrawAllocationDebugger();

        }

        static void DrawAllocationDebugger();
    };

}
