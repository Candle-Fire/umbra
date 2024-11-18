#pragma once

#include <memory>
#include <vector>
#include <shadow/exports.h>
#include <shadow/core/Module.h>

#include "IRenderer.h"
#include "shadow/SHObject.h"

namespace SH::Renderer
{
    API class RenderManager final : public Module
    {
        SHObject_Base(RenderManager)

        IRenderer* mainRenderer = nullptr;

    public:
        void registerRenderer();

        void setRenderer(IRenderer* renderer);
        IRenderer* getRenderer() const;
    };
}
