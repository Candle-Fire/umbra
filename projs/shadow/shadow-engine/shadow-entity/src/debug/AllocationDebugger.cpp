#include "debug/AllocationDebugger.h"

namespace ShadowEngine::Entities::Debugger {

    std::weak_ptr<ShadowEngine::Entities::EntitySystem> AllocationDebugger::entitySystem;

    void AllocationDebugger::DrawAllocationDebugger() {
        if (ImGui::Begin("Allocation Debugger")) {
            if (!entitySystem.expired()) {
                auto a = entitySystem.lock();
                
                auto &mgr = a->GetWorld().GetManager();

                for (auto allocator : mgr.m_NodeContainerRegistry) {
                    ImGui::Text("%i", allocator.first);
                }

            }
        }
        ImGui::End();
    }

}