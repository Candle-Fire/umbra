#pragma once
/*
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

        if (ImGui::Begin("Allocation Debugger")) {

        }
        ImGui::End();

        if (ImGui::Begin("Scene Debugger")) {
            auto scene = entitySystem->GetActiveScene();
            if (scene) {
                ImGui::Text("Scene");
                ImGui::Text("RTM ptr %i", scene.GetInternalUid());
                ImGui::Text("Number of root entities %i", scene->hierarchy.size());
                ImGui::Text("Number of internal root entities %i", scene->internalHierarchy.size());
            } else {
                ImGui::Text("We don't have a scene");
            }
        }
        ImGui::End();
    }
};

}
*/