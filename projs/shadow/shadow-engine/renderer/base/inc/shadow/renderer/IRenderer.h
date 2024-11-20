#pragma once

#include "IRenderTarget.h"
#include "shadow/util/enum_helpers.h"

namespace SH::Renderer {

    enum class RendererCapabilities {
        RENDERER_2D = 1 << 0,
        RENDERER_3D = 1 << 1,
        RENDERER_IMGUI = 1 << 2,
        RENDERER_2D_3D = RENDERER_2D | RENDERER_3D
    };

    class IRenderer {
    public:
        /***
         * Gets the renderer capabilities
         * @return Capabilities
         */
        virtual RendererCapabilities getCapabilities() const = 0;

        virtual void setRenderExtent(int width, int height) = 0;

        /***
         * @brief Returns the active render target
         */
        virtual IRenderTarget& getRenderTarget() const = 0;

        virtual void RenderSceneToTexture() = 0;

        virtual void BeginRenderPass() = 0;
    };
}
