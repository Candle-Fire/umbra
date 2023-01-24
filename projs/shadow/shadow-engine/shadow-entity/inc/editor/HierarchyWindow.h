#pragma once


#include "EntitySystem.h"
#include "imgui.h"
#include "core/ModuleManager.h"

#define ICON_FA_CUBE u8"\uf1b2"

namespace ShadowEngine::Entities::Editor {

    class HierarchyWindow {

        static ShadowEngine::Entities::EntitySystem *entitySystem;

    public:
        static void Draw() {

            if (entitySystem == nullptr)
                entitySystem = ShadowEngine::ModuleManager::instance->GetModuleByType<ShadowEngine::Entities::EntitySystem>();

            DebugHierarchy();

            Inspector();
        }

        static void DebugHierarchy();

        static void Inspector();
    };
}