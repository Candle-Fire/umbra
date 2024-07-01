#pragma once

#include "shadow/entity/EntitySystem.h"
#include "imgui.h"
#include "shadow/core/module-manager-v2.h"
#include "shadow/core/ShadowApplication.h"

#define ICON_FA_CUBE u8"\uf1b2"

namespace SH::Entities::Editor {

    class HierarchyWindow {

        static std::weak_ptr<SH::Entities::EntitySystem> entitySystem;

      public:
        static void Draw() {

            if (entitySystem.expired())
                entitySystem =
                    SH::ShadowApplication::Get().GetModuleManager().GetById<SH::Entities::EntitySystem>(
                        "module:/entity-system");

            DebugHierarchy();

            Inspector();
        }

        static void DebugHierarchy();

        static void Inspector();
    };
}
