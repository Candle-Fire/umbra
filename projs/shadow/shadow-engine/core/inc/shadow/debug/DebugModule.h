#pragma once

#include <SDL_events.h>
#include <imgui.h>
#include "shadow/core/Module.h"
#include "shadow/event-bus/events.h"

namespace SH::Debug {

  class API DebugModule : public Module {

  SHObject_Base(DebugModule)

    bool active;

  public:
    bool w_time = true;
    bool w_modules = true;
    bool w_imguiDemo = true;

    void DrawModuleWindow();

    void DrawTimeWindow();

    void DrawImguiDemo();

    void Init() override;
  };

}
