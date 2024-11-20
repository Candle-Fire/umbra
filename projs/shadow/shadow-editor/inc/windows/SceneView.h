#pragma once

#include <memory>
#include <shadow/renderer/IRenderer.h>

#include "../EditorWindow.h"

namespace SH::Editor {

    class SceneView : public EditorWindow{
    SHObject_Base(SceneView)

     Renderer::IRenderer* renderer;

    public:
        SceneView();

        void Draw() override;
    };

}