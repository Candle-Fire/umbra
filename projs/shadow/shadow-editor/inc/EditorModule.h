#pragma once

#include "core/Module.h"
#include "EditorWindow.h"
#include "core/ShadowApplication.h"
#include "EditorWidget.h"

#include <map>

namespace ShadowEngine::Editor {

    class Menu {
      public:
        std::function<void()> clk;
    };

    class EditorModule : public ShadowEngine::Module {
      SHObject_Base(EditorModule)

        std::vector<std::shared_ptr<EditorWidget>> windows;

        std::map<std::string, Menu> menus;

      public:
        EditorModule() {}

        void RegisterMenu(std::string path, Menu m);

        void EditorRender(SH::Events::EditorRender &);

        void DrawMenu();

      private:
        void Init() override;

    };
}