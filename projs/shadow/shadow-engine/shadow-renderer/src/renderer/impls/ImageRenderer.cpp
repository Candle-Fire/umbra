#include "renderer/ImageRenderer.h"
#include "renderer/Interface.h"
#include "spdlog/spdlog.h"
#include "core/Time.h"
#include "renderer/Renderer.h"
#include "shader/modules/ImageRenderer.h"


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
                    ps.topology = Topology::TRIANGLELIST;
                    iface->CreatePipelineState(&ps, &imagePSO[blend][sten][stenref][GET(StripMode::OFF)]);
                    ps.topology = Topology::TRIANGLESTRIP;
                    iface->CreatePipelineState(&ps, &imagePSO[blend][sten][stenref][GET(StripMode::ON)]);
                }
            }
        }
    }

    void Initialize() {
        rx::Interface* interface = rx::GetInterface();
        double time = Time::timeSinceStart;

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

        spdlog::info("ImageRenderer initialized in " + std::to_string(Time::timeSinceStart - time) + "ms");
    }

    void Draw(const Texture* tex, const RenderMode* mode, ThreadCommands cmd) {
        Interface* interface = rx::GetInterface();
        const Sampler* sample = &samplers[GET(rx::defs::SamplerType::LINEAR_CLAMP)];

        switch (mode->quality) {
            case RenderMode::Quality::NEAREST:
                switch (mode->sampleTiling) {
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
                switch (mode->sampleTiling) {
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
                switch (mode->sampleTiling) {
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
                .texBaseIdx = interface->GetDescriptorIdx(tex, ImageViewType::SHADER_RESOURCE, mode->imageSubResource),
                .texMaskIdx = interface->GetDescriptorIdx(mode->maskMap, ImageViewType::SHADER_RESOURCE, mode->maskSubResource),
                .texBgIdx = mode->isBackground() ? interface->GetDescriptorIdx(&bgTexture, ImageViewType::SHADER_RESOURCE) : -1,
        };

        if (image.samplerIdx < 0) return;

        XMFLOAT4 color = mode->color;
        const float darkening = 1 - mode->fade;
        color.x *= darkening;
        color.y *= darkening;
        color.z *= darkening;
        color.w *= darkening;

        DirectX::XMMATRIX half;



    }

}