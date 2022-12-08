#include "../../inc/windows/DebugWindows.h"
#include "imgui.h"
#include "core/ShadowApplication.h"
#include "../../inc/EditorModule.h"

namespace ShadowEngine::Editor {

    SHObject_Base_Impl(DebugWindows)

    DebugWindows::DebugWindows() {
        debugModule = ShadowEngine::ShadowApplication::Get().GetModuleManager().GetModuleByType<Debug::DebugModule>();

        auto editormodule = ShadowEngine::ShadowApplication::Get().GetModuleManager().GetModuleByType<Editor::EditorModule>();
        editormodule->RegisterMenu("Windows/Modules List", Menu{
            .clk=[this](){
                debugModule->w_modules = true;
        }});
        editormodule->RegisterMenu("Windows/Time info", Menu{
                .clk=[this](){
                    debugModule->w_time = true;
                }});
        editormodule->RegisterMenu("Windows/ImGUI Demo", Menu{
                .clk=[this](){
                    debugModule->w_imguiDemo = true;
                }});
    }

    void DebugWindows::Draw() {
        debugModule->DrawModuleWindow();
        debugModule->DrawTimeWindow();
        debugModule->DrawImguiDemo();
    }
}