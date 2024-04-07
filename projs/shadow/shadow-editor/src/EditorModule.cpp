#include "../inc/EditorModule.h"
#include <imgui.h>
#include <spdlog/spdlog.h>

#include "windows/SceneView.h"
#include "windows/DebugWindows.h"
#include "shadow/util/string-helpers.h"
#include "shadow/core/module-manager-v2.h"

namespace SH::Editor {

  SHObject_Base_Impl(EditorModule)

  MODULE_ENTRY(SH::Editor::EditorModule, EditorModule)

  /*
  void EditorModule::OverlayRender(SH::Events::OverlayRender &) {
      static bool dockspaceOpen = true;

      ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar; //| ImGuiWindowFlags_NoDocking;
      //static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

      ImGuiViewport *viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(viewport->Pos);
      ImGui::SetNextWindowSize(viewport->Size);
      //ImGui::SetNextWindowViewport(viewport->ID);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
          | ImGuiWindowFlags_NoMove;
      window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
      ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
      ImGui::PopStyleVar(3);

      // DockSpace
      ImGuiIO &io = ImGui::GetIO();
      ImGuiStyle &style = ImGui::GetStyle();
      float minWinSizeX = style.WindowMinSize.x;
      style.WindowMinSize.x = 370.0f;
      //if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
      //    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
      //    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
      //}

      if (ImGui::BeginMenuBar()) {
          DrawMenu();

          ImGui::EndMenuBar();
      }

      for (int i = 0; i < windows.size(); ++i) {
          windows[i]->Draw();
      }

      ImGui::End();
  } */

  void EditorModule::DrawMenu() {

      for (const auto &menu : this->menus) {
          std::vector<std::string> menu_path =
              SH::Util::Str::explode(menu.first, '/');

          int depth = 0;
          for (; depth < menu_path.size() - 1; ++depth) {
              if (!ImGui::BeginMenu(menu_path[depth].c_str())) {
                  break;
              }
          }
          if (depth == menu_path.size() - 1)
              if (ImGui::MenuItem(menu_path.back().c_str()))
                  menu.second.clk();

          for (; depth > 0; depth--) {
              ImGui::EndMenu();
          }
      }
  }

  void EditorModule::Init() {
      //SH::ShadowApplication::Get().GetEventBus()
      //    .subscribe(this, &EditorModule::OverlayRender);

      windows.push_back(std::make_shared<SceneView>());
      windows.push_back(std::make_shared<DebugWindows>());
  }

  void EditorModule::RegisterMenu(std::string path, Menu m) {
      menus.emplace(path, m);
  }

}