
#pragma once

#include "../EditorWindow.h"
#include "shadow/debug/DebugModule.h"

namespace SH::Editor {

    class DebugWindows : public EditorWindow {
      SHObject_Base(DebugWindows)

        std::weak_ptr<SH::Debug::DebugModule> debugModule;

      public:
        DebugWindows();

        void Draw() override;
    };

}
