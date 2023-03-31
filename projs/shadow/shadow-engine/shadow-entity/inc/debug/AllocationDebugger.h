#pragma once

#include "imgui.h"
#include "EntitySystem.h"
#include "core/ModuleManager.h"

namespace ShadowEngine::Entities::Debugger {

    class AllocationDebugger {

        static ShadowEngine::Entities::EntitySystem *entitySystem;

      public:
        static void Draw() {

            if (entitySystem == nullptr)
                entitySystem =
                    ShadowEngine::ModuleManager::instance->GetModuleByType<ShadowEngine::Entities::EntitySystem>();

            DrawAllocationDebugger();

        }

        static void DrawAllocationDebugger();
    };

}
