#pragma once

#include "SHObject.h"

namespace ShadowEngine::Editor {

    class EditorWidget : public ShadowEngine::SHObject {
      SHObject_Base(EditorWidget)
      public:
        virtual void Draw()= 0;
    };

}
