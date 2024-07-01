#pragma once

#define GLM_CONSTEXPR_SIMD
#include <cstddef>
#include <vector>
#include <atomic>
#include "Canvas.h"
#include "Interface.h"
#include "shadow/entity/primitive/PrimitiveObjects.h"
#include "shadow/entity/entities/CameraComponent.h"
#include <renderer/GraphicsDefine.h>

#define arraysize(a) (sizeof(a) / sizeof(a[0]))

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

        static constexpr uint32_t CombineStencilRefs(rx::defs::StencilReference engineRef, uint32_t userRef) {
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

        static void ModifySampler(SamplerMeta& meta);

        static void Initialize();

        static void ClearWorld(SH::Entities::World& world);

        static const std::string& GetShaderPath();
        static void SetShaderPath(const std::string& path);
        static const std::string& GetShaderSourcePath();
        static void SetShaderSourcePath(const std::string& path);
        static void ReloadShaders();

        static size_t GetShaderErrorCount();
        static size_t GetShaderMissingCount();

        static bool LoadShader(ShaderStage stage, Shader& out, const std::string& filename, ShaderModel model = ShaderModel::SM60, std::vector<std::string> permute_defines = {});

        struct Visibility {
            uint32_t layerMask = ~0u;
            const SH::Entities::World* world = nullptr;

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
            SH::Entities::World& world,
                const Visibility& vis,
                FrameConstants& fc,
                float dt);

        static void UpdateRenderData(const Visibility& vis, const FrameConstants& fc, ThreadCommands cmd);

        static void UpdateRenderDataAsync(const Visibility& vis, const FrameConstants& fc, ThreadCommands cmd);

        static void UpdateRayTracingStructure(const SH::Entities::World& w, ThreadCommands cmd);

        static void BindCommonResources(ThreadCommands cmd);

        static void BindCamera(const SH::Entities::Builtin::CameraComponent& camera,
                        const SH::Entities::Builtin::CameraComponent& cameraLast,
                        const SH::Entities::Builtin::CameraComponent& cameraReflection,
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

        static void DrawSky(const SH::Entities::World& w, ThreadCommands cmd);

        static void DrawSun(ThreadCommands cmd);

        static void DrawShadowmaps(const Visibility& vis, ThreadCommands cmd);

        static void DrawDebugScene(const SH::Entities::World& world, const SH::Entities::Builtin::CameraComponent& cam, const Canvas& cnv, ThreadCommands cmd);

        static void DrawSoftbodyParticles(const Visibility& vis, const Texture& depth, bool distortion, ThreadCommands cmd);

        static void DrawLightVisualizers(const Visibility& vis, ThreadCommands cmd);

        static void DrawVolumetricLights(const Visibility& vis, ThreadCommands cmd);

        static void DrawLensFlares(const Visibility& vis, ThreadCommands cmd, const Texture* occlusionMap = nullptr);

        static void RefreshEnvironmentProbes(const Visibility& vis, ThreadCommands cmd);

        static void RefreshBillboards(const SH::Entities::World& world, ThreadCommands cmd);

        static void RefreshLightmaps(const SH::Entities::World& world, ThreadCommands cmd, uint8_t instanceOcclusionMask = 0xFF);

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
        static void SurfelGICoverage(const SurfelGIResources& res, const SH::Entities::World& world, const Texture& debug, ThreadCommands cmd);
        static void ComputeSurfelGI(const SurfelGIResources& res, const SH::Entities::World& world, ThreadCommands cmd, uint8_t instanceInclusionMask = 0xFF);

        static void ComputeDDGI(const SH::Entities::World& world, ThreadCommands cmd, uint8_t instanceInclusionMask = 0xFF);

        struct VXGIResources {
            Texture diffuse[2];
            Texture specular[2];
            mutable bool preClear = true;

            bool isValid() const { return diffuse[0].isValid(); }
        };

        static void CreateVXGIResources(VXGIResources& res, DirectX::XMUINT2 resolution);
        static void VXGI_Voxelize(const Visibility& vis, ThreadCommands cmd);

        static void ComputeVXGI(const VXGIResources& res, const SH::Entities::World& world, const Texture& linearDepth, ThreadCommands cmd, bool fullscreen = false);

        static void PostprocessGaussianBlur(const Texture& input, const Texture& temp, const Texture& output, ThreadCommands cmd, int mipSource = -1, int mipDest = -1, bool wide = false);
        static void PostprocessBilateralBlur(const Texture& input, const Texture& linearDepth, const Texture& temp, const Texture& output, ThreadCommands cmd, float depthThreshold = 1.0f, int mipSource = -1, int mipDest = -1, bool wide = false);

        struct SSAOResources {
            Texture temp;
        };

        static void CreateSSAOResources(SSAOResources& res, DirectX::XMUINT2 resolution);
        static void PostprocessSSAO(const SSAOResources& res, const Texture& output, const Texture& linearDepth, ThreadCommands cmd, float range = 1.0f, uint32_t samples = 16, float power = 1.0f);
        static void PostprocessHBAO(const SSAOResources& res, const SH::Entities::Builtin::CameraComponent& camera, const Texture& linearDepth, const Texture& output, ThreadCommands cmd, float power = 1.0f);

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
        static void PostprocessMSAO(const MSAOResources& res, const SH::Entities::Builtin::CameraComponent& camera, const Texture& depth, const Texture& output, ThreadCommands cmd, float power = 1.0f);

        struct RTAOResources {
            Texture normals;
            mutable int frame = 0;
            GPUBuffer tiles;
            GPUBuffer metadata;
            Texture scratch[2];
            Texture moments[2];
        };

        static void CreateRTAOResources(RTAOResources& res, DirectX::XMUINT2 resolution);
        static void PostprocessRTAO(const RTAOResources& res, const SH::Entities::World& world, const Texture& output, ThreadCommands cmd, float range = 1.0f, float power = 1.0f, uint8_t instanceInclusionMask = 0xFF);

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
        static void PostprocessRTDiffuse(const RTDiffuseResources& res, const SH::Entities::World& world, const Texture& output, ThreadCommands cmd, float range = 1000.0f, uint8_t instanceInclusionMask = 0xFF);

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
        static void PostprocessRTReflection(const RTReflectionResources& res, const SH::Entities::World& world, const Texture& output, ThreadCommands cmd, float range = 1000.0f, float roughnessCutoff = 0.5f, uint8_t instanceInclusionMask = 0xFF);

        // ScreenSpace Reflection
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

        struct RTShadowResources {
            Texture temp;
            Texture temporal[2]; // For Temporal anti-aliasing
            Texture normals;

            mutable int frame = 0;
            GPUBuffer tiles;
            GPUBuffer meta;
            Texture scratch[4][2];
            Texture moments[4][2];
            Texture denoise;
        };
        void CreateRTShadowResources(RTShadowResources& res, DirectX::XMUINT2 resolution);
        void PostprocessRTShadow(const RTShadowResources& res, const SH::Entities::Scene& scene, const GPUBuffer& opaqueTiles, const Texture& linearDepth, const Texture& outpit, ThreadCommands cmd, uint8_t instanceMask = 0xFF);

        // ScreenSpace Shadow
        struct SSSResources {
            Texture lowres;
        };
        void CreateSSSResources(SSSResources& res, DirectX::XMUINT2 resolution);
        void PostprocessSSS(const SSSResources& res, const GPUBuffer& opaqueTiles, const Texture& linearDepth, const Texture& output, ThreadCommands cmd, float range = 1, uint32_t samples = 16);

        void PostprocessLightShafts(const Texture& input, const Texture& output, ThreadCommands cmd, const DirectX::XMFLOAT2& center, float strength = 0.1f);

        struct DOFResources {
            Texture maxHorizontal;
            Texture minHorizontal;
            Texture max;
            Texture min;
            Texture maxNeighborhood;
            Texture presort;
            Texture prefilter;
            Texture main;
            Texture postfilter;
            Texture alpha1;
            Texture alpha2;
            GPUBuffer bufferStatistics;
            GPUBuffer bufferEarlyExit;
            GPUBuffer bufferCheap;
            GPUBuffer bufferExpensive;

            bool isValid() const { return maxHorizontal.isValid(); }
        };
        void CreateDOFResources(DOFResources& res, DirectX::XMUINT2 resolution);
        void PostprocessDOF(const DOFResources& res, Texture& input, Texture& output, ThreadCommands cmd, float scale = 10, float max = 10);

        void PostprocessOutline(const Texture& input, ThreadCommands cmd, float threshold = 0.1f, float thickness = 1.0f, const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(0, 0, 0, 1));

        struct MotionBlurResources {
            Texture maxHorizontal;
            Texture minHorizontal;
            Texture max;
            Texture min;
            Texture maxNeighborhood;
            GPUBuffer bufferStatistics;
            GPUBuffer bufferEarlyExit;
            GPUBuffer bufferCheap;
            GPUBuffer bufferExpensive;

            bool isValid() const { return maxHorizontal.isValid(); }
        };
        void CreateMotionBlurResources(MotionBlurResources& res, DirectX::XMUINT2 resolution);
        void PostprocessMotionBlur(const MotionBlurResources& res, const Texture& input, const Texture& output, ThreadCommands cmd, float strength = 100.0f);

        struct AerialPerspectiveResources {
            Texture output;
        };
        void CreateAerialPerspectiveResources(AerialPerspectiveResources& res, DirectX::XMUINT2 resolution);
        void PostprocessAerialPerspective(const AerialPerspectiveResources& res, ThreadCommands cmd);

        // Volumetric Clouds
        struct VCResources {
            mutable int frame = 0;
            DirectX::XMUINT2 finalRes = {};
            Texture cloudRender;
            Texture cloudDepth;
            Texture reprojection[2];
            Texture reprojectionDepth[2];
            Texture reprojectionAdditional[2];
            Texture cloudMask;
        };
        void CreateVCResources(VCResources& res, DirectX::XMUINT2 resolution);
        void PostprocessVC(const VCResources& res, const SH::Entities::Builtin::CameraComponent& camera, const SH::Entities::Builtin::CameraComponent& camPrev, const SH::Entities::Builtin::CameraComponent& camReflect, const bool jitter, ThreadCommands cmd, const Texture* weatherFirst = nullptr, const Texture* weatherSecond = nullptr);
        void PostprocessVCUpsample(const VCResources& res, ThreadCommands cmd);

        void PostprocessFXAA(const Texture& in, const Texture& out, ThreadCommands cmd);

        struct TemporalAAResources {
            mutable int frame = 0;
            Texture temporal[2];

            bool isValid() const { return temporal[0].isValid(); }
            const Texture* getCurrent() const { return &temporal[frame % arraysize(temporal)]; }
            const Texture* getHistory() const { return &temporal[(frame + 1) % arraysize(temporal) ]; }
        };
        void CreateTemporalAAResources(TemporalAAResources& res, DirectX::XMUINT2 resolution);
        void PostprocessTemporalAA(const TemporalAAResources& res, const Texture& in, ThreadCommands cmd);

        void PostprocessSharpen(const Texture& in, const Texture& out, ThreadCommands cmd, float amount = 1.0f);
        void PostprocessTonemap(const Texture& in, const Texture& out, float exposure, float brightness, float contrast, float saturation, bool dither, ThreadCommands cmd, const Texture* gradingLUT = nullptr, const Texture* distortion = nullptr, const GPUBuffer* luminance = nullptr, const Texture* bloom = nullptr, ColorSpace cs = ColorSpace::SRGB);

        void PostprocessFSR(const Texture& in, const Texture& temp, const Texture& out, ThreadCommands cmd, float sharpness = 1.0);

        // Chromatic Aberration
        void PostprocessCA(const Texture& in, const Texture& out, ThreadCommands cmd, float amount = 1.0);

        void PostprocessUpsampleBilat(const Texture& in, const Texture& depth, const Texture& out, ThreadCommands cmd, bool pixel = false, float threshold = 1);
        void PostprocessDownsample4x(const Texture& in, const Texture& out, ThreadCommands cmd);

        void PostprocessNormalsDepth(const Texture& depth, const Texture& out, ThreadCommands cmd);

        void PostprocessUnderwater(const Texture& in, const Texture& out, ThreadCommands cmd);

        void PostprocessCustom(const Shader& compute, const Texture& in, const Texture& out, ThreadCommands cmd, const DirectX::XMFLOAT4& p0 = DirectX::XMFLOAT4(0, 0, 0, 0), const DirectX::XMFLOAT4& p1 = DirectX::XMFLOAT4(0, 0, 0, 0), const char* debugName = "Postprocess_Custom");

        void ConvertVUYToRGB(const Texture& in, int subResourceLuminance, int subResourceChrominance, const Texture& out, ThreadCommands cmd);





        void RaytraceScene(const SH::Entities::Scene& scene, const Texture& out, int accumulation, ThreadCommands cmd, uint8_t instanceMask = 0xFF, const Texture* outAlbedo = nullptr, const Texture* outNormal = nullptr, const Texture* outDepth = nullptr, const Texture* outStencil = nullptr, const Texture* outDepthStencil = nullptr);

        // Start Bounding Volume Hierarchy for the scene
        void RaytraceSceneBVH(const SH::Entities::Scene& scene, ThreadCommands cmd);

        void OcclusionCullReset(const Visibility& vis, ThreadCommands cmd);
        void OcclusionCullRender(const SH::Entities::Builtin::CameraComponent& cam, const Visibility& vis, ThreadCommands cmd);
        void OcclusionCullResolve(const Visibility& vis, ThreadCommands cmd);

        enum MipFilter {
            POINT,
            LINEAR,
            GAUSSIAN
        };

        struct MipOptions {
            int arrayIndex;
            const Texture* temp;
            bool preserveCoverage;
            bool wideGauss;
        };
        void GenerateMipchain(const Texture& tex, MipFilter filter, ThreadCommands cmd, const MipOptions& options = {});

        void BlockCompress(const Texture& in, const Texture& out, ThreadCommands cmd, uint32_t offset = 0);

        enum ExpandStyle {
            NONE,
            WRAP,
            CLAMP
        };

        void CopyTexture(const Texture& dest, int dMIP, int dX, int dY, const Texture& src, int sMIP, int sX, int sY, ThreadCommands cmd, ExpandStyle expand = ExpandStyle::NONE, bool convertsRGB = false);

        void DrawWaterRipples(const Visibility& vis, ThreadCommands cmd);

        void DebugSetShadow2D(int maxRes);
        void DebugSetShadowCube(int maxRes);

#define DEBUG_FEATURE(x) \
        void DebugSet##x(bool val); \
        bool DebugGet##x();
#define DEBUG_FLOAT_FEATURE(x) \
        void DebugSet##x(float val); \
        float DebugGet##x();
#define DEBUG_INT_FEATURE(x) \
        void DebugSet##x(uint32_t val); \
        uint32_t DebugGet##x();

        DEBUG_FEATURE(TransparentShadows);
        DEBUG_FEATURE(WireRender);
        DEBUG_FEATURE(DrawBoneLines);
        DEBUG_FEATURE(DrawPartitions);
        DEBUG_FEATURE(DrawEnvProbes);
        DEBUG_FEATURE(DrawEmitters);
        DEBUG_FEATURE(DrawForcefields);
        DEBUG_FEATURE(DrawCameras);
        DEBUG_FEATURE(DrawColliders);
        DEBUG_FEATURE(DrawGridHelper);
        DEBUG_FEATURE(DrawVoxelHelper);
        DEBUG_FEATURE(LightCulling);
        DEBUG_FEATURE(AdvancedLightCulling);
        DEBUG_FEATURE(VRSClassification);
        DEBUG_FEATURE(VRSClassificationDebug);
        DEBUG_FEATURE(OcclusionCulling);
        DEBUG_FEATURE(TemporalAA);
        DEBUG_FEATURE(TemporalAADebug);
        DEBUG_FEATURE(VXGI);
        DEBUG_FEATURE(VXGIReflections);
        DEBUG_FLOAT_FEATURE(GameSpeed);
        DEBUG_INT_FEATURE(RTBounces);
        DEBUG_FEATURE(RTBVHVisualizer);
        DEBUG_FEATURE(RTShadows);
        DEBUG_FEATURE(Tesselation);
        DEBUG_FEATURE(Allbedo);
        DEBUG_FEATURE(DiffuseLighting);
        DEBUG_FEATURE(ScreenSpaceShadows);
        DEBUG_FEATURE(SurfelGI);
        DEBUG_FEATURE(DDGI);
        DEBUG_INT_FEATURE(DDGIRays);
        DEBUG_FLOAT_FEATURE(DDGIBlend);
        DEBUG_FLOAT_FEATURE(GIBoost);

        void WorkaroundBug(const size_t bug, ThreadCommands cmd);

        void DebugDrawBox(const DirectX::XMFLOAT4& mat, const DirectX::XMFLOAT4& col = DirectX::XMFLOAT4(1, 1, 1, 1));
        void DebugDrawSphere(const rx::Sphere& sphere, const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(1, 1, 1, 1));
        void DebugDrawCapsule(const rx::Capsule& capsule, const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(1, 1, 1, 1));

        struct LineRenderData {
            DirectX::XMFLOAT3 start = DirectX::XMFLOAT3(0, 0, 0);
            DirectX::XMFLOAT3 end = DirectX::XMFLOAT3(0, 0, 0);
            DirectX::XMFLOAT4 colorStart = DirectX::XMFLOAT4(1, 1, 1, 1);
            DirectX::XMFLOAT4 colorEnd = DirectX::XMFLOAT4(1, 1, 1, 1);
        };

        void DebugDrawLine3D(const LineRenderData& data);
        void DebugDrawLine2D(const LineRenderData& data);

        struct PointRenderData {
            DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3(0, 0, 0);
            DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(1, 1, 1, 1);
            float size = 1;
        };

        void DebugDrawPoint(const PointRenderData& point);

        struct TriangleRenderData {
            DirectX::XMFLOAT3 p1Pos = DirectX::XMFLOAT3(0, 0, 0);
            DirectX::XMFLOAT4 p1Col = DirectX::XMFLOAT4(1, 1, 1, 1);
            DirectX::XMFLOAT3 p2Pos = DirectX::XMFLOAT3(0, 0, 0);
            DirectX::XMFLOAT4 p2Col = DirectX::XMFLOAT4(1, 1, 1, 1);
            DirectX::XMFLOAT3 p3Pos = DirectX::XMFLOAT3(0, 0, 0);
            DirectX::XMFLOAT4 p3Col = DirectX::XMFLOAT4(1, 1, 1, 1);
        };

        void DebugRenderTriangle(const TriangleRenderData& tri);

        struct TextRenderData {
            DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3(0, 0, 0);
            int pixelHeight = 32;
            float scale = 1;
            DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(1, 1, 1, 1);
            enum Flags {
                NONE = 0,
                DEPTHTEST = 1<<0,
                BILLBOARD = 1<<1,
                ORTHO = 1<<2
            };
            uint32_t flags = Flags::NONE;
        };

        void DebugDrawText(const char* text, const TextRenderData& data);

        void DeferMipGeneration(const Texture& tex, bool preserve = false);
        void DeferCompression(const Texture& src, const Texture& dst);

        struct CustomShader {
            std::string name;
            uint32_t filterMask = static_cast<uint32_t>(rx::defs::Filter::OPAQUE);
            PipelineState pso[static_cast<uint32_t>(rx::defs::RenderPass::SIZE)];
        };

        int RegisterCustomShader(const CustomShader& shader);
        const std::vector<CustomShader>& GetCustomShaders();

    };
}