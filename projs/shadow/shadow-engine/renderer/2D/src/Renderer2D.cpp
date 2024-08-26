#include "Renderer2D.h"
#include "shadow/SHObject.h"
#include "shadow/renderer/IRenderer.h"

#include <shadow/core/module-manager-v2.h>
#include <shadow/core/SDL2Module.h>
#include <shadow/core/ShadowApplication.h>
#include <shadow/renderer/RenderManager.h>
#include <VK2D/Camera.h>
#include <VK2D/Constants.h>
#include <VK2D/Opaque.h>
#include <VK2D/Renderer.h>
#include <VK2D/Structs.h>
#include "VK2D/Texture.h"

SHObject_Base_Impl(SH::Renderer::V2D::Renderer2D)

MODULE_ENTRY(SH::Renderer::V2D::Renderer2D, Renderer2D)

namespace SH::Renderer::V2D
{
    Renderer2D::Renderer2D()
    = default;

    Renderer2D::~Renderer2D()
    = default;

    SH::Renderer::RendererCapabilities SH::Renderer::V2D::Renderer2D::getCapabilities() const
    {
        return RendererCapabilities::RENDERER_2D | RendererCapabilities::RENDERER_IMGUI;
    }

    void Renderer2D::RecreateRenderTargets() {
        if (render_target != nullptr) {
            vk2dTextureFree(render_target);
        }
        render_target = vk2dTextureCreate(render_target_size.x, render_target_size.y);
    }


    void Renderer2D::setRenderExtent(const int width, const int height)
    {
        render_target_size = glm::ivec2(width, height);
        RecreateRenderTargets();
    }

    void Renderer2D::RenderSceneToTexture()
    {
        renderingToTexture = true;
        RecreateRenderTargets();
    }

    void SH::Renderer::V2D::Renderer2D::PreInit()
    {
        Mgr().IfModuleActive<RenderManager>("module:/render-manager", [this](const std::shared_ptr<RenderManager>& rmg)
        {
            rmg->setRenderer(this);
        });
    }

    void SH::Renderer::V2D::Renderer2D::Init()
    {
        platform = Mgr().GetById<SDL2Module>("module:/platform/sdl2");
        if (platform.expired()) {
            Mgr().DeactivateModule(this);
            return;
        }
        auto p = platform.lock();

        VK2DRendererConfig const config = {VK2D_MSAA_1X, VK2D_SCREEN_MODE_TRIPLE_BUFFER, VK2D_FILTER_TYPE_NEAREST};
        VK2DStartupOptions options = {
            .enableDebug = true,
            .stdoutLogging = true,
            .quitOnError = false,
            .errorFile = "vk2derror.txt",
            .loadCustomShaders = false,
            .vramPageSize = 0
        };
        vk2dRendererInit(p->window->sdlWindowPtr, config, &options);
        if (vk2dStatus() != VK2D_STATUS_NONE) {
            spdlog::error("[Vulkan2D error {0}] {1}", vk2dStatus(), vk2dStatusMessage());
        }


        vk2dColourHex(clearColour, "#FF0000");

        this->setRenderExtent(p->window->Width, p->window->Height);

        cam = {VK2D_CAMERA_TYPE_DEFAULT, 0, 0, p->window->Width * 1.0f, p->window->Height * 1.0f, 1, 0};
        windowCam = vk2dCameraCreate(cam);

        ShadowApplication::Get().GetEventBus().subscribe<WindowResizeEvent>([this](const WindowResizeEvent &PH1) {
            this->setRenderExtent(PH1.Width, PH1.Height);
        });
    }

    void Renderer2D::Update(int frame)
    {
        vk2dRendererStartFrame(clearColour);

        // Draw your things
        // Adjust for window size
        cam.w = (float) render_target_size.x;
        cam.h = (float) render_target_size.y;
        vk2dCameraUpdate(windowCam, cam);
        vk2dRendererLockCameras(windowCam);

        //if (renderingToTexture) {
        //    vk2dRendererSetTarget(render_target);
        //}

        vec4 grey = {0.3, 0.3, 0.3, 1};
        vk2dRendererSetColourMod(VK2D_BLACK);
        vk2dRendererDrawRectangle(50, 50, 100, 100, 0, 0, 0);

        //vec4 clear = {1, 0.0, 0.2, 1.0};
        //vk2dRendererSetBlendMode(VK2D_BLEND_MODE_NONE);
        //vk2dRendererSetTarget(VK2D_TARGET_SCREEN);

        //SH::ShadowApplication::Get().GetEventBus().fire(SH::Renderer::LateRender());

        vk2dRendererEndFrame();
    }

    void Renderer2D::BeginRenderPass()
    {
    }
}