#pragma once

#define GLM_CONSTEXPR_SIMD
#include <cstddef>
#include <vector>
#include <atomic>
#include "Canvas.h"
#include "Interface.h"
#include "graph/graph.h"
#include <entities/CameraComponent.h>
#include <primitive/PrimitiveObjects.h>

namespace rx {

    /**
     * The root of the renderer tree.
     * Defines the functions that need to be implemented for the render to happen.
     * Essentially the renderer "interface".
     */
    class Renderer : public Canvas {
    private:
        uint32_t layerMask = 0xFFFFFFFF;
    public:
        virtual ~Renderer() = default;

        // Load resources that will be needed.
        virtual void Load() {}
        // First time setup things.
        virtual void Start() {}
        // Called when switching to another Renderer
        virtual void Stop() {}
        // Called before the per-frame update
        virtual void PreUpdate() {}
        // Called with fixed frequency - for timer tasks. Ask the engine for the timestamp.
        virtual void FixedUpdate() {}
        // Called when the world is updating after the frame render.
        virtual void Update(float dt) {}
        // Called after update. Bake models or whatever in response to game activities.
        virtual void PostUpdate() {}
        // Render to images
        virtual void Render() const {}
        // Render to canvas
        virtual void Compose(rx::ThreadCommands cmd) const {}

        inline uint32_t GetLayerMask() const { return layerMask; }
        inline void SetLayerMask(uint32_t value) { layerMask = value; }

        rx::ColorSpace colorSpace = ColorSpace::SRGB;

        // Render job methods
        static constexpr ImageFormat depthbufferFormat = ImageFormat::D32_FLOAT_S8X24_UINT;
        static constexpr ImageFormat rendertargetFormat = ImageFormat::R11G11B10_FLOAT;
        static constexpr ImageFormat idBufferFormat = ImageFormat::R32_UINT;
        static constexpr ImageFormat rendertargetShadowmapFormat = ImageFormat::R16G16B16A16_FLOAT;
        static constexpr ImageFormat depthbufferShadowmapFormat = ImageFormat::D16_UNORM;
        static constexpr ImageFormat rendertargetEnvprobeFormat = ImageFormat::R11G11B10_FLOAT;
        static constexpr ImageFormat depthbufferEnvprobeFormat = ImageFormat::D16_UNORM;

        constexpr uint32_t CombineStencilRefs(rx::defs::StencilReference engineRef, uint32_t userRef) {
            return (userRef << 4) | static_cast<uint8_t>(engineRef);
        }

        static const Sampler* GetSampler(rx::defs::SamplerType ID);
        static const Shader* GetShader(rx::defs::ShaderType ID);
        static const InputLayout* GetInputLayout(rx::defs::InputLayout ID);
        static const RasterizerState* GetRasterizerState(rx::defs::RasterizerState ID);
        static const DepthStencilState* GetDepthStencilState(rx::defs::DepthStencilState ID);
        static const BlendState* GetBlendState(rx::defs::BlendModes ID);
        static const GPUBuffer* GetBuffer(rx::defs::BufferType ID);
        static const Texture* GetTexture(rx::defs::TextureType ID);

        static const GPUBuffer& GetIndexBufferForQuads(uint32_t quads);

        static static void ModifySampler(SamplerMeta& meta);

        static static void Initialize();

        static static void ClearWorld(ShadowEngine::Entities::World& world);

        static const std::string& GetShaderPath();
        static static void SetShaderPath(const std::string& path);
        static const std::string& GetShaderSourcePath();
        static static void SetShaderSourcePath(const std::string& path);
        static static void ReloadShaders();

        static size_t GetShaderErrorCount();
        static size_t GetShaderMissingCount();

        static bool LoadShader(ShaderStage stage, Shader& out, const std::string& filename, ShaderModel model = ShaderModel::SM60, std::vector<std::string> permute_defines = {});

        struct Visibility {
            uint32_t layerMask = ~0u;
            const ShadowEngine::Entities::World* world = nullptr;
            const ShadowEngine::Entities::Builtin::CameraComponent* camera = nullptr;

            enum Flags {
                EMPTY = 0,
                ALLOW_OBJECTS = 1,
                ALLOW_LIGHTS = 2,
                ALLOW_DECALS = 4,
                ALLOW_ENVPROBES = 8,
                ALLOW_EMITTERS = 16,
                ALLOW_HAIRS = 32,
                ALLOW_REQUEST_REFLECTIONS = 64,
                ALLOW_OCCLUSION_CULLING = 128,

                ALLOW_EVERYTHING = ~0
            };

            uint32_t flags = EMPTY;
            rx::Frustum frustum;
            std::vector<uint32_t> visibleObjects;
            std::vector<uint32_t> visibleDecals;
            std::vector<uint32_t> visibleEnvProbes;
            std::vector<uint32_t> visibleEmitters;
            std::vector<uint32_t> visibleHairs;
            std::vector<uint32_t> visibleLights;

            std::atomic<uint32_t> objectCounter;
            std::atomic<uint32_t> lightCounter;
            std::atomic<uint32_t> decalCounter;

            // TODO: syncronization?
            bool reflectionVisible = false;
            float closestReferencePlane = std::numeric_limits<float>::max();
            DirectX::XMFLOAT4 reflectionPlane = DirectX::XMFLOAT4(0, 1, 0, 0);
            std::atomic_bool requestedVolumetricLighting { false };

            void Clear() {
                visibleObjects.clear();
                visibleDecals.clear();
                visibleLights.clear();
                visibleEnvProbes.clear();
                visibleEmitters.clear();
                visibleHairs.clear();

                objectCounter.store(0);
                lightCounter.store(0);
                decalCounter.store(0);

                closestReferencePlane = std::numeric_limits<float>::max();
                reflectionVisible = false;
                requestedVolumetricLighting.store(false);
            }

            bool RequestedPlanarReflections() const {
                return reflectionVisible;
            }

            bool RequestedVolumetricLighting() const {
                return requestedVolumetricLighting.load();
            }
        };

        struct FrameConstants;

        static void UpdateVisibility(Visibility& vis);

        static void UpdatePerFrameData(
                ShadowEngine::Entities::World& world,
                const Visibility& vis,
                FrameConstants& fc,
                float dt);

        static void UpdateRenderData(const Visibility& vis, const FrameConstants& fc, ThreadCommands cmd);

        static void UpdateRenderDataAsync(const Visibility& vis, const FrameConstants& fc, ThreadCommands cmd);

        static void UpdateRayTracingStructure(const ShadowEngine::Entities::World& w, ThreadCommands cmd);

        static void BindCommonResources(ThreadCommands cmd);

        static void BindCamera(const ShadowEngine::Entities::Builtin::CameraComponent& camera,
                        const ShadowEngine::Entities::Builtin::CameraComponent& cameraLast,
                        const ShadowEngine::Entities::Builtin::CameraComponent& cameraReflection,
                        ThreadCommands cmd);

        enum DrawSceneFlags {
            DRAW_OPAQUE = 1,
            DRAW_TRANSPARENT = 2,
            DRAW_OCCLUSION_CULLING = 4,
            DRAW_TESSELATION = 8,
            DRAW_HAIR = 16,
            DRAW_BILLBOARD = 32,
            DRAW_OCEAN = 64,
            DRAW_SKIP_PLANAR_REFLECTIONS = 128
        };

        static void DrawScene(const Visibility& vis, rx::defs::RenderPass renderPass, ThreadCommands cmd, uint32_t flags = DRAW_OPAQUE);

        static void ProcessDeferredRequests(ThreadCommands cmd);

        static void ComputeVolumetricCloudShadows(ThreadCommands cmd, const Texture* map1 = nullptr, const Texture* map2 = nullptr);

        static void ComputeAtmosphericTextures(ThreadCommands cmd);

        static void ComputeAtmosphericLut(ThreadCommands cmd);

        static void ComputeAtmosphericCameraVolume(ThreadCommands cmd);

        static void DrawSky(const ShadowEngine::Entities::World& w, ThreadCommands cmd);

        static void DrawSun(ThreadCommands cmd);

        static void DrawShadowmaps(const Visibility& vis, ThreadCommands cmd);

        static void DrawDebugScene(const ShadowEngine::Entities::World& world, const ShadowEngine::Entities::Builtin::CameraComponent& cam, const Canvas& cnv, ThreadCommands cmd);

        static void DrawSoftbodyParticles(const Visibility& vis, const Texture& depth, bool distortion, ThreadCommands cmd);

        static void DrawLightVisualizers(const Visibility& vis, ThreadCommands cmd);

        static void DrawVolumetricLights(const Visibility& vis, ThreadCommands cmd);

        static void DrawLensFlares(const Visibility& vis, ThreadCommands cmd, const Texture* occlusionMap = nullptr);

        static void RefreshEnvironmentProbes(const Visibility& vis, ThreadCommands cmd);

        static void RefreshBillboards(const ShadowEngine::Entities::World& world, ThreadCommands cmd);

        static void RefreshLightmaps(const ShadowEngine::Entities::World& world, ThreadCommands cmd, uint8_t instanceOcclusionMask = 0xFF);

        static void ResolveMSAA(const Texture& dest, const Texture& source, ThreadCommands cmd);

        static void DownsampleDepthBuffer(const Texture& dest, ThreadCommands cmd);

        struct TiledLightResources {
            DirectX::XMUINT3 tileCount = {};
            GPUBuffer tileFrustums;
            GPUBuffer tilesOpaque;
            GPUBuffer tilesTransparent;
        };

        static void CreateTiledLightResources(TiledLightResources& res, DirectX::XMUINT2 resolution);

        static void ComputeTiledLightCulling(const TiledLightResources& res, Texture& debugUAV, ThreadCommands cmd);

        struct LuminanceResources {
            GPUBuffer luminance;
        };

        static void CreateLuminanceResources(LuminanceResources& res, DirectX::XMUINT2 resolution);

        static void ComputeLuminance(const LuminanceResources& res, Texture& source, ThreadCommands cmd, float adaption = 1, float eyeadaption = 0.115f);

        struct BloomResources {
            Texture bloomTexture;
            Texture temp;
        };

        static void CreateBloomResources(BloomResources& res, DirectX::XMUINT2 resolution);

        static void ComputeBloom(const BloomResources& res, Texture& input, ThreadCommands cmd, float threshold = 1.0f, float exposure = 1.0f, const GPUBuffer* luminanceBuffer = nullptr);

        static void ComputeShadingRateClassification(const Texture& output, const Texture& debug, ThreadCommands cmd);

        struct VisibilityResources {
            DirectX::XMUINT2 tileCount = {};
            GPUBuffer bins;
            GPUBuffer binnedTiles;
            Texture payload0;
            Texture payload1;
            Texture normals;
            Texture roughness;

            const Texture* depthBuffer = nullptr;
            const Texture* linearDepth = nullptr;
            const Texture* primitiveResolved = nullptr;
        };

        static void CreateVisibilityResources(VisibilityResources& res, DirectX::XMUINT2 resolution);

        static void VisibilityPrepare(const VisibilityResources& res, Texture& input, ThreadCommands cmd);

        static void VisibilitySurface(const VisibilityResources& res, Texture& output, ThreadCommands cmd);

        static void VisibilitySurfaceReduced(const VisibilityResources& res, ThreadCommands cmd);

        static void VisibilityShade(const VisibilityResources& res, Texture& output, ThreadCommands cmd);

        static void VisibilityVelocity(const VisibilityResources& res, Texture& output, ThreadCommands cmd);

        struct SurfelGIResources {
            Texture result;
        };

        static void CreateSurfelGIResources(SurfelGIResources& res, DirectX::XMUINT2 resolution);
        static void SurfelGICoverage(const SurfelGIResources& res, const ShadowEngine::Entities::World& world, const Texture& debug, ThreadCommands cmd);
        static void ComputeSurfelGI(const SurfelGIResources& res, const ShadowEngine::Entities::World& world, ThreadCommands cmd, uint8_t instanceInclusionMask = 0xFF);

        static void ComputeDDGI(const ShadowEngine::Entities::World& world, ThreadCommands cmd, uint8_t instanceInclusionMask = 0xFF);

        struct VXGIResources {
            Texture diffuse[2];
            Texture specular[2];
            mutable bool preClear = true;

            bool isValid() const { return diffuse[0].isValid(); }
        };

        static void CreateVXGIResources(VXGIResources& res, DirectX::XMUINT2 resolution);
        static void VXGI_Voxelize(const Visibility& vis, ThreadCommands cmd);

        static void ComputeVXGI(const VXGIResources& res, const ShadowEngine::Entities::World& world, const Texture& linearDepth, ThreadCommands cmd, bool fullscreen = false);

        static void PostprocessGaussianBlur(const Texture& input, const Texture& temp, const Texture& output, ThreadCommands cmd, int mipSource = -1, int mipDest = -1, bool wide = false);
        static void PostprocessBilateralBlur(const Texture& input, const Texture& linearDepth, const Texture& temp, const Texture& output, ThreadCommands cmd, float depthThreshold = 1.0f, int mipSource = -1, int mipDest = -1, bool wide = false);

        struct SSAOResources {
            Texture temp;
        };

        static void CreateSSAOResources(SSAOResources& res, DirectX::XMUINT2 resolution);
        static void PostprocessSSAO(const SSAOResources& res, const Texture& output, const Texture& linearDepth, ThreadCommands cmd, float range = 1.0f, uint32_t samples = 16, float power = 1.0f);
        static void PostprocessHBAO(const SSAOResources& res, const ShadowEngine::Entities::Builtin::CameraComponent& camera, const Texture& linearDepth, const Texture& output, ThreadCommands cmd, float power = 1.0f);

        struct MSAOResources {
            Texture depthDownsize1;
            Texture depthTiled1;
            Texture depthDownsize2;
            Texture depthTiled2;
            Texture depthDownsize3;
            Texture depthTiled3;
            Texture depthDownsize4;
            Texture depthTiled4;
            Texture aoMerged1;
            Texture aoHQ1;
            Texture aoSmooth1;
            Texture aoMerged2;
            Texture aoHQ2;
            Texture aoSmooth2;
            Texture aoMerged3;
            Texture aoHQ3;
            Texture aoSmooth3;
            Texture aoMerged4;
            Texture aoHQ4;
        };

        static void CreateMSAOResources(MSAOResources& res, DirectX::XMUINT2 resolution);
        static void PostprocessMSAO(const MSAOResources& res, const ShadowEngine::Entities::Builtin::CameraComponent& camera, const Texture& depth, const Texture& output, ThreadCommands cmd, float power = 1.0f);

        struct RTAOResources {
            Texture normals;
            mutable int frame = 0;
            GPUBuffer tiles;
            GPUBuffer metadata;
            Texture scratch[2];
            Texture moments[2];
        };

        static void CreateRTAOResources(RTAOResources& res, DirectX::XMUINT2 resolution);
        static void PostprocessRTAO(const RTAOResources& res, const ShadowEngine::Entities::World& world, const Texture& output, ThreadCommands cmd, float range = 1.0f, float power = 1.0f, uint8_t instanceInclusionMask = 0xFF);

        struct RTDiffuseResources {
            mutable int frame = 0;
            Texture rayIndirectDiffuse;
            Texture spatial;
            Texture spatialVariance;
            Texture temporal[2];
            Texture temporalVariance[2];
            Texture bilateralTemp;
        };

        static void CreateRTDiffuseResources(RTDiffuseResources& res, DirectX::XMUINT2 resolution);
        static void PostprocessRTDiffuse(const RTDiffuseResources& res, const ShadowEngine::Entities::World& world, const Texture& output, ThreadCommands cmd, float range = 1000.0f, uint8_t instanceInclusionMask = 0xFF);

        struct RTReflectionResources {
            mutable int frame = 0;
            Texture rayIndirectDiffuse;
            Texture rayDirection;
            Texture rayLengths;
            Texture resolve;
            Texture resolveVariance;
            Texture resolveReprojection;
            Texture temporal[2];
            Texture temporalVariance[2];
            Texture bilateralTemp;
        };

        static void CreateRTReflectionResources(RTReflectionResources& res, DirectX::XMUINT2 resolution);
        static void PostprocessRTReflection(const RTReflectionResources& res, const ShadowEngine::Entities::World& world, const Texture& output, ThreadCommands cmd, float range = 1000.0f, float roughnessCutoff = 0.5f, uint8_t instanceInclusionMask = 0xFF);

        struct SSRResources {
            mutable int frame = 0;
            Texture roughnessHorizontal;
            Texture roughness;
            Texture depth;
            Texture raySpecular;
            Texture rayDir;
            Texture rayLength;
            Texture resolve;
            Texture resolveVariance;
            Texture resolveReprojection;
            Texture temporal[2];
            Texture temporalVariance[2];
            Texture bilateralTemp;
            GPUBuffer tracingStats;
            GPUBuffer tracingFast;
            GPUBuffer tracingCheap;
            GPUBuffer tracingSlow;
        };
        void CreateSSRResources(SSRResources& res, DirectX::XMUINT2 resolution);
        void PostprocessSSR(const SSRResources& res, const Texture& input, const Texture& output, ThreadCommands cmd, float cutoff = 0.6f);



    };
}