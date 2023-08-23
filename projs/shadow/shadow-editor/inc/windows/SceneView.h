#pragma once

#include "../EditorWidget.h"
#include "vlkx/vulkan/VulkanModule.h"

namespace ShadowEngine::Editor {

    class SceneView : public EditorWidget {
    SHObject_Base(SceneView)

    public:
        SceneView();

        void Draw() override;
    };

}