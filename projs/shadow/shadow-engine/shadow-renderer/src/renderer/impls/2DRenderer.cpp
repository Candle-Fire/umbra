#include "renderer/impls/2DRenderer.h"
#include "renderer/Interface.h"
#include "renderer/ImageRenderer.h"

namespace rx {
    void Renderer2D::ResizeBuffers() {
        currentBufferSize = GetInternalResolution();
        currentLayoutScale = 0;

        Interface* interface = rx::GetInterface();

        // Figure out if this is the first time setup (depth stencil null)
        const Texture* depthBuffer = GetDepthStencil();
        if (depthBuffer != nullptr && (resScale != 1.0f || depthBuffer->getMeta().sampleCount > 1)) {
            TextureMeta meta = depthBuffer->getMeta();
            meta.layout = ResourceState::SHADER_RESOURCE;
            meta.bindFlag = BindFlag::RENDER_TARGET | BindFlag::SHADER_RESOURCE;
            meta.format = ImageFormat::R8G8B8A8_UNORM;

            interface->CreateTexture(&meta, nullptr, &targetStenciled);
            interface->SetName(&targetStenciled, "Stenciled Target");

            if (meta.sampleCount > 1) {
                meta.sampleCount = 1;
                interface->CreateTexture(&meta, nullptr, &targetStencilResolved);
                interface->SetName(&targetStencilResolved, "Resolved Stenciled Target");
            }
        } else {
            targetStenciled = {}; // Delete
        }

        TextureMeta meta {
            .width = GetPhysicalWidth(),
            .height = GetPhysicalHeight(),
            .format = ImageFormat::R8G8B8A8_UNORM,
            .bindFlag = BindFlag::RENDER_TARGET | BindFlag::SHADER_RESOURCE
        };

        interface->CreateTexture(&meta, nullptr, &target);
        interface->SetName(&target, "Target");
    }

    void Renderer2D::ResizeLayout() {
        currentLayoutScale = GetDPIScaling();
    }

    void Renderer2D::Update(float dt) {
        glm::vec2 internalRes = GetInternalResolution();

        if (currentBufferSize.x != internalRes.x || currentBufferSize.y != internalRes.y)
            ResizeBuffers();
        if (currentLayoutScale != GetDPIScaling())
            ResizeLayout();

        // GUI Update?

        for (auto& layer : layers) {
            for (auto& renderable : layer.renderables) {
                switch(renderable.type) {
                    default:
                    case Renderable2D::Type::SPRITE:
                        if (renderable.sprite != nullptr)
                            renderable.sprite->Update(dt);
                        break;
                    case Renderable2D::Type::ATLAS:
                        if (renderable.atlas != nullptr)
                            renderable.atlas->Update(dt);
                        break;
                }
            }
        }

        Renderer::Update(dt);
    }

    void Renderer2D::FixedUpdate() {
        for (auto& layer : layers) {
            for (auto& renderable : layer.renderables) {
                switch(renderable.type) {
                    default:
                    case Renderable2D::Type::SPRITE:
                        if (renderable.sprite != nullptr)
                            renderable.sprite->FixedUpdate();
                        break;
                    case Renderable2D::Type::ATLAS:
                        if (renderable.atlas != nullptr)
                            renderable.atlas->FixedUpdate();
                        break;
                }
            }
        }

        Renderer::FixedUpdate();
    }

    void Renderer2D::Render() const {
        Interface* interface = GetInterface();
        ThreadCommands commands = interface->BeginCommands();

        Renderer::ProcessDeferredRequests(commands);

        const Texture* dsv = GetDepthStencil();

        if (targetStenciled.isValid()) {
            if (targetStenciled.getMeta().sampleCount > 1) {
                RenderPassImage img[] = {
                        RenderPassImage::RenderTarget(&targetStenciled, RenderPassImage::LoadOp::CLEAR),
                        RenderPassImage::DepthStencil(dsv, RenderPassImage::LoadOp::LOAD,
                                                      RenderPassImage::StoreOp::STORE, ResourceState::DEPTH_STENCIL_RO,
                                                      ResourceState::DEPTH_STENCIL_RO, ResourceState::DEPTH_STENCIL_RO),
                        RenderPassImage::Resolve(&targetStencilResolved)
                };

                interface->BeginRenderPass(img, 3, commands);

            } else {
                RenderPassImage imgs[] = {
                        RenderPassImage::RenderTarget(&targetStenciled, RenderPassImage::LoadOp::CLEAR),
                        RenderPassImage::DepthStencil(dsv, RenderPassImage::LoadOp::LOAD,
                                                      RenderPassImage::StoreOp::STORE, ResourceState::DEPTH_STENCIL_RO,
                                                      ResourceState::DEPTH_STENCIL_RO, ResourceState::DEPTH_STENCIL_RO),
                };

                interface->BeginRenderPass(imgs, 2, commands);
            }

            dsv = nullptr;

            Viewport vp{
                    .width = (float) targetStenciled.getMeta().width,
                    .height = (float) targetStenciled.getMeta().height
            };

            interface->BindViewports(&vp, 1, commands);

            interface->EventBegin("Stencil sprite layers", commands);
            for (auto &layer: layers) {
                for (auto &renderable: layer.renderables) {
                    if (renderable.type = Renderable2D::Type::SPRITE && renderable.sprite != nullptr &&
                                          renderable.sprite->params.stencilComp != rx::Image::StencilDisabled)
                        renderable->Draw(commands);
                }
            }
            interface->EventEnd(commands);

            interface->EndRenderPass(commands);
        }
        if (dsv != nullptr && !targetStenciled.isValid()) {
            RenderPassImage imgs[] = {
                    RenderPassImage::RenderTarget(&target, RenderPassImage::LoadOp::CLEAR),
                    RenderPassImage::DepthStencil(dsv, RenderPassImage::LoadOp::LOAD, RenderPassImage::StoreOp::STORE, ResourceState::DEPTH_STENCIL_RO, ResourceState::DEPTH_STENCIL_RO, ResourceState::DEPTH_STENCIL_RO)
            };

            interface->BeginRenderPass(imgs, 2, commands);
        } else {
            RenderPassImage img = RenderPassImage::RenderTarget(&target, RenderPassImage::LoadOp::CLEAR);
            interface->BeginRenderPass(&img, 1, commands);
        }

        Viewport port {
            .width = (float) target.getMeta().width,
            .height = (float) target.getMeta().height
        };

        interface->BindViewports(&port, 1, commands);

        if (GetDepthStencil() != nullptr) {
            if (targetStenciled.isValid()) {
                interface->EventBegin("Copy stenciled sprite layers");

            }
        }
    }

}