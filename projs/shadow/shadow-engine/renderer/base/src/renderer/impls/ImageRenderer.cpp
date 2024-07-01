#include "renderer/ImageRenderer.h"
#include "renderer/Interface.h"
#include "spdlog/spdlog.h"
#include "shadow/core/Time.h"
#include "renderer/Renderer.h"
#include "shader/modules/ImageRenderer.h"
#include <directxpackedvector.h>


#define GET(x) static_cast<uint32_t>(x)

namespace rx::Image {

    static Sampler samplers[GET(rx::defs::SamplerType::SIZE)];
    static Shader vertexShader;
    static Shader fragmentShader;
    static rx::BlendState blendState[GET(rx::defs::BlendModes::SIZE)];
    static RasterizerState rasterState;
    static DepthStencilState depthStencilStates[GET(rx::ComparisonFunc::SIZE)][GET(rx::defs::StencilMask::SIZE)];
    enum class StripMode : uint32_t {
        OFF,
        ON,
        SIZE
    };
    static PipelineState imagePSO[GET(rx::defs::BlendModes::SIZE)][GET(rx::ComparisonFunc::SIZE)][GET(rx::defs::StencilMask::SIZE)][GET(StripMode::SIZE)];
    static thread_local Texture bgTexture;
    static thread_local Canvas canvas;

    void SetBackground(const Texture& tex) {
        bgTexture = tex;
    }

    void SetCanvas(const Canvas& cvs) {
        canvas = cvs;
    }

    void LoadShaders() {
        rx::Renderer::LoadShader(ShaderStage::VS, vertexShader, "image.vs");
        rx::Renderer::LoadShader(ShaderStage::FS, fragmentShader, "image.fs");
        Interface* iface = rx::GetInterface();

        PipelineStateMeta ps {
            .vertex = &vertexShader,
            .fragment = &fragmentShader,
            .rasterizer = &rasterState
        };

        for (int blend = 0; blend < GET(rx::defs::BlendModes::SIZE); blend++) {
            ps.blend = &blendState[blend];
            for (int sten = 0; sten < GET(rx::ComparisonFunc::SIZE); sten++) {
                for (int stenref = 0; stenref < GET(rx::defs::StencilMask::SIZE); stenref++) {
                    ps.depthStencil = &depthStencilStates[sten][stenref];
                    for (int strip = 0; strip < GET(StripMode::SIZE); strip++) {
                        switch (strip) {
                            case GET(StripMode::ON):
                                ps.topology = Topology::TRIANGLESTRIP;
                                break;
                            case GET(StripMode::OFF):
                                ps.topology = Topology::TRIANGLELIST;
                                break;
                        }
                        iface->CreatePipelineState(&ps, &imagePSO[blend][sten][stenref][strip]);
                    }
                }
            }
        }
    }

    void Initialize() {
        SH::Timer timer;
        rx::Interface* interface = rx::GetInterface();

        RasterizerState s {
            .fillMode = rx::DrawMode::SOLID,
            .culling = rx::CullingMode::NONE,
            .reverseWindingOrder = false,
            .depthBias = 0,
            .depthBiasClamp = 0,
            .slopeDepthBias = 0,
            .depthClipping = true,
            .multisample = false,
            .antialiasing = false
        };
        rasterState = s;

        for (int i = 0; i < GET(rx::defs::StencilMask::SIZE); i++) {
            DepthStencilState ds {
                .depth = false,
                .stencil = false
            };

            depthStencilStates[GET(rx::ComparisonFunc::NEVER)][i] = ds;
            ds.stencil = true;
            switch (i) {
                case GET(rx::defs::StencilMask::ENGINE):
                    ds.stencil_read = GET(rx::defs::StencilMask::ENGINE);
                    break;
                case GET(rx::defs::StencilMask::USER):
                    ds.stencil_read = GET(rx::defs::StencilMask::USER);
                    break;
                default:
                    ds.stencil_read = GET(rx::defs::StencilMask::ALL);
                    break;
            }

            ds.stencil_write = 0;
            ds.front = {
                    .fail = StencilOp::KEEP,
                    .depthFail = StencilOp::KEEP,
                    .pass = StencilOp::KEEP,
            };
            ds.back = {
                    .fail = StencilOp::KEEP,
                    .depthFail = StencilOp::KEEP,
                    .pass = StencilOp::KEEP,
            };

            for (auto & depthStencilState : depthStencilStates) {
                ds.front.stencil = static_cast<rx::ComparisonFunc>(i);
                ds.back.stencil = static_cast<rx::ComparisonFunc>(i);
                depthStencilState[i] = ds;
            }
        }

        BlendState::RenderTargetBlend rtb {
            .enable = true,
            .source = BlendData::SOURCE_ALPHA,
            .dest = BlendData::SOURCE_ALPHA_INVERSE,
            .op = BlendOp::ADD,
            .sourceAlpha = BlendData::ONE,
            .destAlpha = BlendData::SOURCE_ALPHA_INVERSE,
            .opAlpha = BlendOp::ADD,
            .writeMask = ColorWrite::ENABLE_ALL
        };
        rx::BlendState blend;
        blend.independent = false;
        blend.targets[0] = rtb;
        blendState[GET(defs::BlendModes::ALPHA)] = blend;
        blend.targets[0].source = BlendData::ONE;
        blendState[GET(defs::BlendModes::PREMULTIPLIED)] = blend;
        blend.targets[0].enable = false;
        blendState[GET(defs::BlendModes::OPAQUE)] = blend;
        blend.targets[0].enable = true;
        blend.targets[0].source = BlendData::SOURCE_ALPHA;
        blend.targets[0].dest = BlendData::ONE;
        blend.targets[0].sourceAlpha = BlendData::CLEAR;
        blendState[GET(defs::BlendModes::ADDITIVE)] = blend;
        blend.targets[0].source = BlendData::CLEAR,
        blend.targets[0].dest = BlendData::SOURCE_COLOR;
        blend.targets[0].sourceAlpha = BlendData::CLEAR;
        blend.targets[0].destAlpha = BlendData::SOURCE_ALPHA;
        blendState[GET(defs::BlendModes::MULTIPLY)] = blend;


        SamplerMeta sam {
            .maxAnisotropy = 0,
            .comparison = ComparisonFunc::NEVER,
            .borderColor = SamplerBorderColor::TRANSPARENT_BLACK,
            .minLOD = 0,
            .maxLOD = std::numeric_limits<float>::max(),
            .mipLODBias = 0
        };

        // Skip OBJECT_SHADER, COMPARE_DEPTH
        for (int i = 1; i < GET(rx::defs::SamplerType::COMPARE_DEPTH); i++) {
            sam.filter = i < GET(rx::defs::SamplerType::POINT_CLAMP) ? Filtering::MIN_MAG_MIP_LINEAR : i < GET(rx::defs::SamplerType::ANISO_CLAMP) ? Filtering::MIN_MAG_MIP_POINT : Filtering::ANISOTROPIC;
            sam.maxAnisotropy = i > GET(rx::defs::SamplerType::POINT_MIRROR) ? 16 : 0;
            sam.uTiling = static_cast<ImageTiling>(i);
            sam.vTiling = static_cast<ImageTiling>(i);
            sam.wTiling = static_cast<ImageTiling>(i);
            interface->CreateSampler(&sam, &samplers[i]);
        }

        LoadShaders();

        spdlog::info("ImageRenderer initialized in " + std::to_string(timer.elapsedMillis()) + "ms");
    }

    void Draw(const Texture* tex, const RenderMode& mode, ThreadCommands cmd) {
        Interface* interface = rx::GetInterface();
        const Sampler* sample = &samplers[GET(rx::defs::SamplerType::LINEAR_CLAMP)];

        switch (mode.quality) {
            case RenderMode::Quality::NEAREST:
                switch (mode.sampleTiling) {
                    case ImageTiling::MIRROR:
                        sample = &samplers[GET(rx::defs::SamplerType::POINT_MIRROR)];
                        break;
                    case ImageTiling::WRAP:
                        sample = &samplers[GET(rx::defs::SamplerType::POINT_WRAP)];
                        break;
                    case ImageTiling::CLAMP:
                        sample = &samplers[GET(rx::defs::SamplerType::POINT_CLAMP)];
                    default:
                        break;
                }
                break;

            case RenderMode::Quality::LINEAR:
                switch (mode.sampleTiling) {
                    case ImageTiling::MIRROR:
                        sample = &samplers[GET(rx::defs::SamplerType::LINEAR_MIRROR)];
                        break;
                    case ImageTiling::WRAP:
                        sample = &samplers[GET(rx::defs::SamplerType::LINEAR_WRAP)];
                        break;
                    case ImageTiling::CLAMP:
                        sample = &samplers[GET(rx::defs::SamplerType::LINEAR_CLAMP)];
                    default:
                        break;
                }
                break;

            case RenderMode::Quality::ANISO:
                switch (mode.sampleTiling) {
                    case ImageTiling::MIRROR:
                        sample = &samplers[GET(rx::defs::SamplerType::ANISO_MIRROR)];
                        break;
                    case ImageTiling::WRAP:
                        sample = &samplers[GET(rx::defs::SamplerType::ANISO_WRAP)];
                        break;
                    case ImageTiling::CLAMP:
                        sample = &samplers[GET(rx::defs::SamplerType::ANISO_CLAMP)];
                    default:
                        break;
                }
                break;

            default:
                break;
        }

        ImageConstants image = {
                .bufferIdx = -1,
                .bufferOffset = 0,
                .samplerIdx = interface->GetDescriptorIdx(sample),
                .texBaseIdx = interface->GetDescriptorIdx(tex, ImageViewType::SHADER_RESOURCE, mode.imageSubResource),
                .texMaskIdx = interface->GetDescriptorIdx(mode.maskMap, ImageViewType::SHADER_RESOURCE, mode.maskSubResource),
                .texBgIdx = mode.isBackground() ? interface->GetDescriptorIdx(&bgTexture, ImageViewType::SHADER_RESOURCE) : -1,
        };

        if (image.samplerIdx < 0) return;

        XMFLOAT4 color = mode.color;
        const float darkening = 1 - mode.fade;
        color.x *= darkening;
        color.y *= darkening;
        color.z *= darkening;
        color.w *= mode.opacity;

        color.x *= mode.intensity;
        color.y *= mode.intensity;
        color.z *= mode.intensity;

        DirectX::PackedVector::XMHALF4 packed;
        packed.x = DirectX::PackedVector::XMConvertFloatToHalf(color.x);
        packed.y = DirectX::PackedVector::XMConvertFloatToHalf(color.y);
        packed.z = DirectX::PackedVector::XMConvertFloatToHalf(color.z);
        packed.w = DirectX::PackedVector::XMConvertFloatToHalf(color.w);

        image.packedColor.x = uint(packed.v);
        image.packedColor.y = uint(packed.v >> 32);

        image.flags = 0;
        if (mode.isExtractNormalMap())
            image.flags |= IMAGE_FLAG_EXTRACT_NORMALMAP;
        if (mode.isHDR())
            image.flags |= IMAGE_FLAG_COLORSPACE_HDR;
        if (mode.isLinear()) {
            image.flags |= IMAGE_FLAG_COLORSPACE_LINEAR;
            image.hdrScaling = mode.HDRScaling;
        }
        if (mode.isFullscreen())
            image.flags |= IMAGE_FLAG_FULLSCREEN;

        image.soften = mode.borderSoften;
        image.alphaMask = DirectX::PackedVector::XMConvertFloatToHalf(mode.maskLowerLimit) | (DirectX::PackedVector::XMConvertFloatToHalf(mode.maskUpperLimit) << 16);

        StripMode strip = StripMode::ON;
        size_t indexCount = 0;

        if (mode.isFullscreen()) {
            image.bufferIdx = -1;
            image.bufferOffset = 0;
        } else {
            DirectX::XMMATRIX S = DirectX::XMMatrixScaling(mode.scale.x * mode.size.x, mode.scale.y * mode.size.y, 1);
            DirectX::XMMATRIX M = DirectX::XMMatrixRotationZ(mode.rotation);

            if (mode.customRotate != nullptr)
                M = M * (*mode.customRotate);
            M = M * DirectX::XMMatrixTranslation(mode.pos.x, mode.pos.y, mode.pos.z);
            if (mode.customProjection != nullptr) {
                S = XMMatrixScaling(1, -1, 1) * S; // Reverse screen projection
                M = M * (*mode.customProjection);
            } else {
                assert(canvas.width > 0);
                assert(canvas.height > 0);
                assert(canvas.DPI > 0);
                M = M * canvas.GetProjection();
            }

            DirectX::XMVECTOR V[4];
            float4 corners[4];

            for (int i = 0; i < arraysize(mode.corners); i++) {
                V[i] = DirectX::XMVectorSet(mode.corners[i].x - mode.pivot.x, mode.corners[i].y - mode.pivot.y, 0, 1);
                V[i] = DirectX::XMVector2Transform(V[i], S);
                DirectX::XMStoreFloat4(corners + i, DirectX::XMVector2Transform(V[i], M));
            }

            image.b0 = float2(corners[0].x, corners[0].y);
            image.b1 = float2(corners[1].x - corners[0].x, corners[1].y - corners[0].y);
            image.b2 = float2(corners[2].x - corners[0].x, corners[2].y - corners[0].y);
            image.b3 = float2(corners[0].x - corners[1].x - corners[2].x + corners[3].x, corners[0].y - corners[1].y - corners[2].y + corners[3].y);

            Interface::GPUAllocation memory = interface->CreateStagingBuffer(sizeof(float4) * 4, cmd);
            image.bufferIdx = interface->GetDescriptorIdx(&memory.buffer, ImageViewType::SHADER_RESOURCE);
            image.bufferOffset = (uint) memory.offset;
            std::memcpy(memory.data, corners, sizeof(corners));
        }

        if (mode.isMirror())
            image.flags |= IMAGE_FLAG_MIRRORED;

        float invHeight = 1;
        float invWidth = 1;
        if (tex != nullptr) {
            const TextureMeta& meta = tex->getMeta();
            invWidth = 1 / float(meta.width);
            invHeight = 1 / float(meta.height);
        }

        if (mode.isDrawRect()) {
            image.texMulAdd.x = mode.drawRect.z * invWidth;
            image.texMulAdd.y = mode.drawRect.w * invHeight;
            image.texMulAdd.z = mode.drawRect.x * invWidth;
            image.texMulAdd.w = mode.drawRect.y * invHeight;
        } else {
            image.texMulAdd = DirectX::XMFLOAT4(1, 1, 0, 0);
        }

        image.texMulAdd.z += mode.texOffset.x * invWidth;
        image.texMulAdd.w += mode.texOffset.y * invHeight;

        if (mode.isDrawRect2()) {
            image.texMulAdd2.x = mode.drawRect2.z * invWidth;
            image.texMulAdd2.y = mode.drawRect2.w * invHeight;
            image.texMulAdd2.z = mode.drawRect2.x * invWidth;
            image.texMulAdd2.w = mode.drawRect2.y * invHeight;
        } else {
            image.texMulAdd2 = DirectX::XMFLOAT4(1, 1, 0, 0);
        }

        image.texMulAdd.z += mode.texOffset2.x * invWidth;
        image.texMulAdd.w += mode.texOffset2.y * invHeight;

        interface->EventBegin("Image Draw", cmd);

        uint32_t stencil = mode.stencil;
        if (mode.stencilMask == defs::StencilMask::USER)
            stencil = rx::Renderer::CombineStencilRefs(defs::StencilReference::EMPTY, (uint8_t) stencil);

        interface->BindStencil(stencil, cmd);

        interface->BindPSO(&imagePSO[static_cast<uint32_t>(mode.stencilBlend)][static_cast<uint32_t>(mode.stencilCompare)][static_cast<uint32_t>(mode.isDepthTest())][static_cast<uint32_t>(strip)], cmd);

        interface->BindDynamicConstants(image, BUFFER_IMAGE, cmd);

        if (mode.isFullscreen())
            interface->Draw(3, 0, cmd);
        else {
            switch (strip) {
                case rx::Image::StripMode::OFF:
                    interface->DrawIndexed(indexCount, 0, 0, cmd);
                    break;
                case rx::Image::StripMode::ON:
                    interface->Draw(4, 0, cmd);
                    break;
            }
        }

        interface->EventEnd(cmd);
    }

}