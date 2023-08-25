#pragma once

#include "SDL_events.h"
#include "core/Module.h"
#include "event-bus/events.h"
#include "imgui.h"

namespace SH::Debug {

    class DebugModule : public Module {

      SHObject_Base(DebugModule)

        bool active;

      public:
        bool w_time = true;
        bool w_modules = true;
        bool w_imguiDemo = true;

        void DrawDirect(SH::Events::OverlayRender &);

        void DrawModuleWindow();

        void DrawTimeWindow();

        void DrawImguiDemo();

        void Init() override;
    };

}
