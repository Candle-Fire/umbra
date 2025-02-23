#pragma once

#include <memory>
#include "../EditorWindow.h"
#include "shadow/renderer/vlkx/vulkan/VulkanModule.h"

namespace SH::Editor {

    class SceneView : public EditorWindow{
    SHObject_Base(SceneView)

    std::shared_ptr<VulkanModule> renderer;

    public:
        SceneView();

        void Draw() override;
    };

}