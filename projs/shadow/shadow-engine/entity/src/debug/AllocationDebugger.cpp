#include "shadow/entity/debug/AllocationDebugger.h"
#include "shadow/entity/EntitySystem.h"

namespace SH::Entities::Debugger {

    std::weak_ptr<SH::Entities::EntitySystem> AllocationDebugger::entitySystem;

    void AllocationDebugger::DrawAllocationDebugger() {
        if (ImGui::Begin("Allocation Debugger")) {
            if (!entitySystem.expired()) {
                auto a = entitySystem.lock();

                auto &mgr = a->GetWorld().GetManager();

                for (auto allocator : mgr.m_NodeContainerRegistry) {
                    ImGui::Text("%s : %i", allocator.second->getTypeName().c_str(), allocator.second->getCount());
                }

            }
        }
        ImGui::End();
    }

}