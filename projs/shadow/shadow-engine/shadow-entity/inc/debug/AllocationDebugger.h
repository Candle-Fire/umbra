#pragma once

#include "imgui.h"
#include "EntitySystem.h"
#include "core/ModuleManager.h"

namespace ShadowEngine::Entities::Debugger {

    class AllocationDebugger  {

        static

    public:
        static void Draw(){

            if(ImGui::Begin("Allocation Debugger")){

                ShadowEngine::ModuleManager::instance->GetModuleByType<EntitySystem>()



            }
            ImGui::End();
        }
    };

}
