#pragma once

namespace SH::Renderer {

    enum ImGuiMode
    {
        /**
        * Draws the Imgui window first and only draws the scene view to a back buffer
        */
        Host,
        /**
        * Draws the scene to the swap chain only allowing floating windows in imgui
        */
        Normal,
    };


    class IImGuiRenderer {
    public:
        virtual ~IImGuiRenderer() = default;

        virtual void EnableImGui() = 0;

        virtual void FrameStart() = 0;

        virtual void FrameEnd() = 0;

        /**
         * @brief Returns the way ImGui is drawn by the renderer backend
         */
        virtual ImGuiMode getImGuiMode() = 0;
        virtual void setImGuiMode(ImGuiMode mode) = 0;
    };
}