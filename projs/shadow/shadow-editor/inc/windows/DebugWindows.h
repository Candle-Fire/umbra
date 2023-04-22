
#pragma once

#include "../EditorWindow.h"
#include "debug/DebugModule.h"

namespace ShadowEngine::Editor {

    class DebugWindows : public EditorWindow {
      SHObject_Base(DebugWindows)

        std::weak_ptr<ShadowEngine::Debug::DebugModule> debugModule;

      public:
        DebugWindows();

        void Draw() override;
    };

}
