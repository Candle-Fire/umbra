#include <renderer/impls/2DRenderer.h>
#include <renderer/Interface.h>
#include <renderer/ImageRenderer.h>
#include <renderer/interfaces/Sprite.h>
#include <renderer/interfaces/FontSprite.h>

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
                    case Renderable2D::Type::FONT:
                        if (renderable.font != nullptr)
                            renderable.font->Update(dt);
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
                    case Renderable2D::Type::FONT:
                        if (renderable.font != nullptr)
                            renderable.font->FixedUpdate();
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
                    if (renderable.type == Renderable2D::Type::SPRITE && renderable.sprite != nullptr &&
                                          renderable.sprite->params.stencilCompare != rx::ComparisonFunc::NEVER)
                        renderable.sprite->Draw(commands);
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
                interface->EventBegin("Copy stenciled sprite layers", commands);
                rx::Image::RenderMode fx;
                fx.enableFullscreen();
                if (targetStenciled.getMeta().sampleCount > 1)
                    rx::Image::Draw(&targetStencilResolved, fx, commands);
                else
                    rx::Image::Draw(&targetStenciled, fx, commands);
                interface->EventEnd(commands);
            } else {
                interface->EventBegin("Stencil Sprite Layers", commands);
                for (auto& x : layers) {
                    for (auto& y : x.renderables) {
                        if (y.type == Renderable2D::Type::SPRITE && y.sprite != nullptr && y.sprite->params.stencilCompare != rx::ComparisonFunc::NEVER)
                            y.sprite->Draw(commands);
                    }
                }
                interface->EventEnd(commands);
            }
        }

        interface->EventBegin("Sprite layers", commands);
        for (auto& x : layers) {
            for (auto& y : x.renderables) {
                switch (y.type) {
                    default:
                    case Renderable2D::Type::SPRITE:
                        if (y.sprite != nullptr && y.sprite->params.stencilCompare == rx::ComparisonFunc::NEVER)
                            y.sprite->Draw(commands);
                        break;
                    case Renderable2D::Type::FONT:
                        if (y.font != nullptr)
                            y.font->Draw(commands);
                        break;
                }
            }
        }
        interface->EventEnd(commands);
        interface->EndRenderPass(commands);
        Renderer::Render();
    }

    void Renderer2D::Compose(rx::ThreadCommands cmd) const {
        rx::Image::RenderMode fx;
        fx.enableFullscreen();
        fx.stencilBlend = defs::BlendModes::PREMULTIPLIED;
        if (colorSpace != ColorSpace::SRGB)
            fx.enableLinear(HDRScaling);
        rx::Image::Draw(&GetRenderResult(), fx, cmd);
        Renderer::Compose(cmd);
    }

    void Renderer2D::AddSprite(rx::Sprite *s, const std::string &layer) {
        for (auto& x : layers) {
            if (!x.layerName.compare(layer)) {
                Renderable2D r2;
                r2.type = Renderable2D::Type::SPRITE;
                r2.sprite = s;
                x.renderables.emplace_back(r2);
            }
        }
        SortLayers();
    }

    void Renderer2D::RemoveSprite(rx::Sprite *s) {
        for (auto& x : layers) {
            for (auto& y : x.renderables) {
                if (y.type == Renderable2D::Type::SPRITE && y.sprite == s)
                    y.sprite = nullptr;
            }
        }
        ClearLayers();
    }

    void Renderer2D::ClearSprites() {
        for (auto& x : layers) {
            for (auto& y : x.renderables) {
                if (y.type == Renderable2D::Type::SPRITE)
                    y.sprite = nullptr;
            }
        }
        ClearLayers();
    }

    int Renderer2D::GetSpriteOrder(rx::Sprite *s) {
        for (auto& x : layers) {
            for (auto& y : x.renderables) {
                if (y.sprite == s)
                    return y.order;
            }
        }
        return 0;
    }

    void Renderer2D::AddLayer(const std::string &name) {
        for (auto& x : layers) {
            if (!x.layerName.compare(name)) return;
        }

        RenderLayer2D layer;
        layer.layerName = name;
        layer.order = layers.size();
        layers.emplace_back(layer);
        layers.back().renderables.clear();
    }

    void Renderer2D::SetLayerOrder(const std::string &name, int order) {
        for (auto& x : layers) {
            if (!x.layerName.compare(name)) {
                x.order = order;
                break;
            }
        }

        SortLayers();
    }

    void Renderer2D::SetSpriteOrder(rx::Sprite *s, int order) {
        for (auto& x : layers) {
            for (auto& y : x.renderables) {
                if (y.type == Renderable2D::Type::SPRITE && y.sprite == s)
                    y.order = order;
            }
        }

        SortLayers();
    }

    void Renderer2D::SortLayers() {
        if (layers.empty()) return;

        for (size_t i = 0; i < layers.size() - 1; i++) {
            for (size_t j = i + 1; j < layers.size(); j++) {
                if (layers[i].order > layers[j].order) {
                    RenderLayer2D swap = layers[i];
                    layers[i] = layers[j];
                    layers[j] = swap;
                }
            }
        }

        for (auto& x : layers) {
            if (x.renderables.empty()) continue;

            for (size_t i = 0; i < x.renderables.size() - 1; i++) {
                for (size_t j = i + 1; j < x.renderables.size(); j++) {
                    if (x.renderables[i].order > x.renderables[j].order) {
                        Renderable2D swap = x.renderables[i];
                        x.renderables[i] = x.renderables[j];
                        x.renderables[j] = swap;
                    }
                }
            }
        }
    }

    void Renderer2D::ClearLayers() {
        for (auto& x : layers) {
            if (x.renderables.empty()) continue;

            std::vector<Renderable2D> toRetain;
            for (auto& y : x.renderables) {
                if (y.sprite != nullptr || y.font != nullptr)
                    toRetain.push_back(y);
            }
            x.renderables.clear();
            x.renderables = toRetain;
        }
    }
}