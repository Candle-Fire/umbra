#include "../../inc/windows/DebugWindows.h"
#include "imgui.h"
#include "core/ShadowApplication.h"

namespace ShadowEngine::Editor {

    SHObject_Base_Impl(DebugWindows)

    DebugWindows::DebugWindows() {
        debugModule = ShadowEngine::ShadowApplication::Get().GetModuleManager().GetModuleByType<Debug::DebugModule>();
    }

    void DebugWindows::Draw() {
        debugModule->DrawModuleWindow();
        debugModule->DrawTimeWindow();
        debugModule->DrawImguiDemo();
    }

    void DebugWindows::AddMenu() {
        if (ImGui::MenuItem("Modules"))
            debugModule->w_modules = true;

        if (ImGui::MenuItem("Time"))
            debugModule->w_time = true;

        if (ImGui::MenuItem("ImGUI Demo"))
            debugModule->w_imguiDemo = true;
    }


}