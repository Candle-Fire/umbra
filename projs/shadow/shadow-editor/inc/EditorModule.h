#pragma once

#include "core/Module.h"
#include "EditorWindow.h"

#include <map>

namespace ShadowEngine::Editor {

    class Menu {
      public:
        std::function<void()> clk;
    };

    class EditorModule : public ShadowEngine::Module {
      SHObject_Base(EditorModule)

        std::vector<std::shared_ptr<EditorWindow>> windows;

        std::map<std::string, Menu> menus;

      public:
        EditorModule() {}

        void RegisterMenu(std::string path, Menu m);

        void OverlayRender();

        void DrawMenu();

      private:
        void Init() override;

    };
}