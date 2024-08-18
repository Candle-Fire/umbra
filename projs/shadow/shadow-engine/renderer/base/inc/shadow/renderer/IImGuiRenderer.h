#pragma once

namespace SH::Renderer {
    class IImGuiRenderer {
    public:
        virtual void InitImGui() = 0;

        virtual void FrameStart() = 0;

        virtual void FrameEnd() = 0;
    };
}