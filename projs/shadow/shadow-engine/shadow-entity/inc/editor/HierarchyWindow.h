#pragma once

#include "EntitySystem.h"
#include "imgui.h"
#include "core/module-manager-v2.h"
#include "core/ShadowApplication.h"

#define ICON_FA_CUBE u8"\uf1b2"

namespace ShadowEngine::Entities::Editor {

    class HierarchyWindow {

        static std::weak_ptr<ShadowEngine::Entities::EntitySystem> entitySystem;

      public:
        static void Draw() {

            if (entitySystem.expired())
                entitySystem =
                    ShadowEngine::ShadowApplication::Get().GetModuleManager().GetById<ShadowEngine::Entities::EntitySystem>(
                        "module:/entity-system");

            DebugHierarchy();

            Inspector();
        }

        static void DebugHierarchy();

        static void Inspector();
    };
}
