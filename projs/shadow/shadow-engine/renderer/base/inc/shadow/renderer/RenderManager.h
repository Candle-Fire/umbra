#pragma once
#include <memory>
#include <string>
#include <vector>
#include <shadow/exports.h>
#include <shadow/core/Module.h>

#include "IRenderer.h"

namespace SH::Renderer
{
    API class RenderManager: public Module
    {
        SHObject_Base(RenderManager)

        std::weak_ptr<IRenderer> mainRenderer;

    public:
        void addRenderer();

        void setRenderer();

        std::vector<IRenderer*> getRenderers();
    };
}
