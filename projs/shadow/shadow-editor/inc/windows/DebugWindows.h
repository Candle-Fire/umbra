
#pragma once

#include "../EditorWidget.h"
#include "debug/DebugModule.h"

namespace ShadowEngine::Editor {

  class DebugWindows : public EditorWidget {
      SHObject_Base(DebugWindows)

        std::weak_ptr<ShadowEngine::Debug::DebugModule> debugModule;

      public:
        DebugWindows();

        void Draw();
    };

}
