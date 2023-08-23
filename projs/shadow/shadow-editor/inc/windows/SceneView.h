#pragma once

#include "../EditorWidget.h"
#include "vlkx/vulkan/VulkanModule.h"

namespace ShadowEngine::Editor {

    class SceneView : public EditorWidget {
    SHObject_Base(SceneView)

        std::vector<VkDescriptorSet> gameImguiTextures;
    public:
        SceneView();

        void Draw() override;
    };

}