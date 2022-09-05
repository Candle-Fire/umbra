//
// Created by dpete on 31/08/2022.
//

#include "debug/DebugModule.h"
#include "imgui.h"
#include "core/Time.h"
#include "core/ModuleManager.h"

void ShadowEngine::Debug::DebugModule::Render() {

    ImGui::Begin("Time", &active, ImGuiWindowFlags_MenuBar);

    ImGui::Text("delta time in ms: %lf", Time::deltaTime_ms);
    ImGui::Text("delta time in s: %lf", Time::deltaTime);
    ImGui::Text("LAST time in: %ld", Time::LAST);

    ImGui::End();

    ImGui::Begin("Active Modules", &active, ImGuiWindowFlags_MenuBar);

    ShadowEngine::ModuleManager* m = ShadowEngine::ModuleManager::instance;



    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "Active Modules:");
    for (auto& module : m->modules)
    {
        ImGui::Text("%s", module.module->GetName().c_str());
    }


    ImGui::End();

}
