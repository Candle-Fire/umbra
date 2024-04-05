#pragma once

#include "../EditorWindow.h"
#include "shadow/renderer/vulkan/vlkx/vulkan/VulkanModule.h"

namespace SH::Editor {

    class SceneView : public EditorWindow{
    SHObject_Base(SceneView)

    VulkanModule* renderer;

    public:
        SceneView();

        void Draw() override;
    };

}