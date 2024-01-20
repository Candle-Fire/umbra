#include "../../inc/windows/DebugWindows.h"
#include "imgui.h"
#include "shadow/core/ShadowApplication.h"
#include "../../inc/EditorModule.h"

namespace SH::Editor {

    SHObject_Base_Impl(DebugWindows)

    DebugWindows::DebugWindows() {
        ShadowApplication &application = SH::ShadowApplication::Get();

        debugModule = application.GetModuleManager().GetById<Debug::DebugModule>("module:/debug");

        auto e = application.GetModuleManager().GetById<Editor::EditorModule>("module:/editor");
        if (e.expired())
            return;

        auto editormodule = e.lock();

        editormodule->RegisterMenu("Windows/Modules List", Menu{
            .clk=[this]() {
                debugModule.lock()->w_modules = true;
            }});
        editormodule->RegisterMenu("Windows/Time info", Menu{
            .clk=[this]() {
                debugModule.lock()->w_time = true;
            }});
        editormodule->RegisterMenu("Windows/ImGUI Demo", Menu{
            .clk=[this]() {
                debugModule.lock()->w_imguiDemo = true;
            }});
    }

    void DebugWindows::Draw() {
        debugModule.lock()->DrawModuleWindow();
        debugModule.lock()->DrawTimeWindow();
        debugModule.lock()->DrawImguiDemo();
    }
}