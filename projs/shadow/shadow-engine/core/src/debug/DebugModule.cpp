#include "debug/DebugModule.h"
#include "imgui.h"
#include "core/Time.h"
#include "core/module-manager-v2.h"
#include "core/ShadowApplication.h"

SHObject_Base_Impl(SH::Debug::DebugModule)

MODULE_ENTRY(SH::Debug::DebugModule, DebugModule)

void SH::Debug::DebugModule::DrawTimeWindow() {

    if (!w_time)
        return;

    if (ImGui::Begin("Time", &w_time, ImGuiWindowFlags_MenuBar)) {
        ImGui::Text("Time since start: %lf", Time::deltaTime_ms);
        ImGui::Text("Delta time in ms: %lf", Time::deltaTime);
    }

    ImGui::End();
}

void SH::Debug::DebugModule::DrawModuleWindow() {

    if (!w_modules)
        return;

    if (ImGui::Begin("Active Modules", &w_modules, ImGuiWindowFlags_MenuBar)) {

        SH::ModuleManager &m = SH::ShadowApplication::Get().GetModuleManager();

        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "Active Modules:", 0);
        for (auto &module : m.GetModules()) {
            if (module.enabled)
                ImGui::Text("%s", module.descriptor.name.c_str());
            else
                ImGui::TextDisabled("%s", module.descriptor.name.c_str());
        }

    }

    ImGui::End();
}

void SH::Debug::DebugModule::DrawImguiDemo() {

    if (w_imguiDemo)
        ImGui::ShowDemoWindow(&w_imguiDemo);

}

void SH::Debug::DebugModule::Init() {
    SH::ShadowApplication::Get().GetEventBus().subscribe(
        this,
        &DebugModule::DrawDirect
    );
}

void SH::Debug::DebugModule::DrawDirect(SH::Events::OverlayRender &) {
    //this->DrawModuleWindow();
    //this->DrawImguiDemo();
    //this->DrawTimeWindow();
}
