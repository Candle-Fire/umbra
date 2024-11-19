#pragma once

#include <glm/vec2.hpp>
#include <shadow/core/Module.h>
#include <shadow/core/SDL2Module.h>
#include <VK2D/Structs.h>

#include "shadow/SHObject.h"
#include "shadow/renderer/IRenderer.h"
#include "shadow/renderer/IImGuiRenderer.h"

namespace SH::Renderer::V2D
{

    class Renderer2D final : public Module, public IRenderer, public IImGuiRenderer
    {
        SHObject_Base(Renderer2D)

        std::weak_ptr<SDL2Module> platform;

        vec4 clearColour;

        bool renderingToTexture = false;
        VK2DTexture render_target = nullptr;
        glm::ivec2 render_target_size;

        VK2DCameraIndex windowCam;
        VK2DCameraSpec cam;

        bool ImGuiEnabled = false;
    public:
        Renderer2D();

        ~Renderer2D() override;

        RendererCapabilities getCapabilities() const override;
        void RecreateRenderTargets();

        void setRenderExtent(int width, int height) override;
        void RenderSceneToTexture() override;
        void BeginRenderPass() override;

        void PreInit() override;
        void Init() override;
        void Update(int frame) override;


        // #####
        // ImGui
        // #####

        ImGuiMode imguiMode = ImGuiMode::Normal;

        void InitImGUI() override;
        void ImGuiFrameStart() override;
        void ImGuiFrameEnd() override;
        ImGuiMode getImGuiMode() override;
        void setImGuiMode(ImGuiMode mode) override;
    };

}

