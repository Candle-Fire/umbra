#pragma once

#include "shadow/core/Module.h"
#include "EditorWindow.h"
#include "shadow/core/ShadowApplication.h"

#include <map>

namespace SH::Editor {

    class Menu {
      public:
        std::function<void()> clk;
    };

    class EditorModule : public SH::Module {
      SHObject_Base(EditorModule)

        std::vector<std::shared_ptr<EditorWindow>> windows;

        std::map<std::string, Menu> menus;

      public:
        EditorModule() {}

        void RegisterMenu(std::string path, Menu m);

        void OverlayRender(SH::Events::OverlayRender &);

        void DrawMenu();

      private:
        void Init() override;

    };
}