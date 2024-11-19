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

    /**
    * Should be implemented by renderers that support drawing ImGUI
    */
    class IImGuiRenderer {
    public:
        virtual ~IImGuiRenderer() = default;

        virtual void InitImGUI() = 0;

        virtual void ImGuiFrameStart() = 0;

        virtual void ImGuiFrameEnd() = 0;

        /**
         * @brief Returns the way ImGui is drawn by the renderer backend
         */
        virtual ImGuiMode getImGuiMode() = 0;
        virtual void setImGuiMode(ImGuiMode mode) = 0;
    };

}
