#pragma once

#include <shadow/core/Module.h>

#include "shadow/renderer/IRenderer.h"

namespace SH::Renderer::V2D
{

    class Renderer2D : public Module, IRenderer
    {
        SHObject_Base(Renderer2D)

    public:
        Renderer2D();

        virtual ~Renderer2D();

        RendererCapabilities getCapabilities() const override;

        void setRenderExtent(int width, int height) override;
        void RenderSceneToTexture() override;
        void BeginRenderPass() override;

        void PreInit() override;
        void Init() override;
        void Update(int frame) override;
    };

}

