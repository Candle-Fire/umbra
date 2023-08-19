#pragma once

#include "../EditorWindow.h"
#include "vlkx/vulkan/VulkanModule.h"

namespace ShadowEngine::Editor {

    class SceneView : public EditorWindow{
    SHObject_Base(SceneView)

    std::shared_ptr<VulkanModule> renderer;

    public:
        SceneView();

        void Draw() override;
    };

}