#include "debug/DebugModule.h"
#include "imgui.h"
#include "core/Time.h"
#include "core/module-manager-v2.h"
#include "core/ShadowApplication.h"

SHObject_Base_Impl(ShadowEngine::Debug::DebugModule)

MODULE_ENTRY(ShadowEngine::Debug::DebugModule, DebugModule)

void ShadowEngine::Debug::DebugModule::OverlayRender() {

    if (ImGui::Begin("Time", &active, ImGuiWindowFlags_MenuBar)) {
        ImGui::Text("delta time in ms: %lf", Time::deltaTime_ms);
        ImGui::Text("delta time in s: %lf", Time::deltaTime);
        ImGui::Text("LAST time in: %d", Time::LAST);
    }

    ImGui::End();

    if (ImGui::Begin("Active Modules", &active, ImGuiWindowFlags_MenuBar)) {

        ShadowEngine::ModuleManager &m = ShadowEngine::ShadowApplication::Get().GetModuleManager();

        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "Active Modules:");
        for (auto &module: m.GetModules()) {
            if(module.enabled)
                ImGui::Text("%s", module.descriptor.name.c_str());
            else
                ImGui::TextDisabled("%s", module.descriptor.name.c_str());
        }

    }

    ImGui::End();

}