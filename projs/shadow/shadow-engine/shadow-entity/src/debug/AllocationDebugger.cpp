#include "debug/AllocationDebugger.h"

namespace ShadowEngine::Entities::Debugger {

    EntitySystem *AllocationDebugger::entitySystem = nullptr;

    void AllocationDebugger::DrawAllocationDebugger() {
        if (ImGui::Begin("Allocation Debugger")) {
            auto &mgr = entitySystem->GetWorld().GetManager();

            for (auto allocator : mgr.m_NodeContainerRegistry) {
                ImGui::Text("%i", allocator.first);
            }

        }
        ImGui::End();
    }

}