#pragma once

/**
 * Holds a way to render the game world.
 * Convenient, huh?
 */

#include "core/Module.h"
#include "vlkx/vulkan/abstraction/Image.h"
#include "vlkx/render/render_pass/ScreenRenderPass.h"

namespace vlkx {
    class GameRenderer : public ShadowEngine::Module {
        SHObject_Base(GameRenderer)

        std::vector<std::unique_ptr<vlkx::Image>> gameOutput;
        std::unique_ptr<vlkx::RenderCommand> renderCommands;
        std::unique_ptr<vlkx::ScreenRenderPassManager> renderPass;

      public:
        void Render();
        void Init();

        const std::vector<std::unique_ptr<vlkx::Image>>& getOutput() { return gameOutput; }
    };
}