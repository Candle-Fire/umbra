//
// Created by dpete on 07/12/2022.
//
#pragma once

#include "SHObject.h"
#include "vlkx/vulkan/abstraction/Image.h"
#include "vlkx/vulkan/abstraction/Commands.h"
#include "vlkx/render/render_pass/ScreenRenderPass.h"
#include "vlkx/vulkan/VulkanModule.h"
#include <vector>
#include <memory>
#include <vulkan/vulkan.hpp>


namespace ShadowEngine::Editor {

    class EditorWindow : public ShadowEngine::SubmitterModule {
    SHObject_Base(EditorWindow)

        std::unique_ptr<vlkx::RenderCommand> renderCommands;
        std::unique_ptr<vlkx::ScreenRenderPassManager> editorPass;

    public:
        void Draw() override;
        void Init() override;
        glm::vec2 GetRenderExtent() override;
    };

}
