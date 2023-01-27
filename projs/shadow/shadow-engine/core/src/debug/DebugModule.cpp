#include "debug/DebugModule.h"
#include "imgui.h"
#include "core/Time.h"
#include "core/ModuleManager.h"

SHObject_Base_Impl(ShadowEngine::Debug::DebugModule)

void ShadowEngine::Debug::DebugModule::OverlayRender() {

    if (ImGui::Begin("Time", &active, ImGuiWindowFlags_MenuBar)) {
        ImGui::Text("delta time in ms: %lf", Time::deltaTime_ms);
        ImGui::Text("delta time in s: %lf", Time::deltaTime);
        ImGui::Text("LAST time in: %d", Time::LAST);
    }

    ImGui::End();

    if (ImGui::Begin("Active Modules", &active, ImGuiWindowFlags_MenuBar)) {

        ShadowEngine::ModuleManager *m = ShadowEngine::ModuleManager::instance;

        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "Active Modules:");
        for (auto &module: m->modules) {
            ImGui::Text("%s", module.module->GetId().c_str());
        }
    }

    ImGui::End();

}