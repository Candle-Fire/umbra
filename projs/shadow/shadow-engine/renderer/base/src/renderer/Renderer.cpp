
#include "shader/modules/Renderer.h"

#include <renderer/Renderer.h>
#include <shadow/util/Synchronization.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <shader/compiler/ShaderCompiler.h>
#include <shadow/core/PathID.h>
#include <shadow/core/Time.h>
#include <shadow/core/jobs/Job.h>
#include <shadow/util/string-helpers.h>

#include "spdlog/spdlog.h"

/**
* A bunch of internal states used by the Renderer abstractions (2D, 3D, PT, PTGI), abstracted through Vulkan and DX interfaces.
*/
namespace rx::internal {

    Interface*& iface = GetInterface();

#define ENUMSIZE(x) static_cast<uint32_t>(x::SIZE)

    Shader              shaders[ENUMSIZE(defs::ShaderType)];
    Texture             texs[ENUMSIZE(defs::TextureType)];
    InputLayout         layouts[ENUMSIZE(defs::InputLayout)];
    RasterizerState     rasters[ENUMSIZE(defs::RasterizerState)];
    DepthStencilState   stencils[ENUMSIZE(defs::DepthStencilState)];
    BlendState          blends[ENUMSIZE(defs::BlendModes)];
    GPUBuffer           buffers[ENUMSIZE(defs::BufferType)];
    Sampler             samplers[ENUMSIZE(defs::SamplerType)];

    /**
     * An internally (externally?) tracked array of memory barriers
     */
    static thread_local std::vector<BarrierType> barriers;
    /**
     * Process all stored barrier commands in the above list.
     * @param cmd a valid ThreadCommands object
     */
    void FlushBarriers(ThreadCommands cmd) {
        if (barriers.empty()) return;
        iface->Barrier(barriers.data(), barriers.size(), cmd);
        barriers.clear();
    }

    /*  ================================================================================================================== */
    /*  Renderer State */
    /*  Note: Most of the bools here are intended to be set by the editor, and read by the scene loader to set on startup. */

    std::unique_ptr<SH::FileSystem> fs;                                                                       // The active filesystem, to load assets and files from.
    SH::Path shaderSourcePath;                                                                                          // The Filesystem path where shader source files are located. Expected to be within the above filesystem.
    SH::Path shaderBinaryPath;                                                                                          // The Filesystem path where shader binary files are located. Must be within the above filesystem. Must be writable. Shaders whose source is available but no binary stored will have their binary created.

    std::atomic_bool ready { false };                                                                                  // Whether the renderer has initialized and is ready to operate.

    float timescale = 1;                                                                                                // How fast things change. A multiplier for deltaTime, to make animations speed up correctly.

    bool renderShadowsOfTransparentObjects = true;                                                                      // Whether transparent objects (such as glass spheres) should cast shadows on objects. This is expected to always be true.
    bool renderDebugWireframes = false;                                                                                 // Debug: render wireframes of solids, rather than shaded polygons.
    bool renderDebugBones = false;                                                                                      // Debug: render animation bones over the top of rigged meshes.
    bool renderDebugPartitions = false;                                                                                 // Debug: render binary space partitions as brushes.
    bool renderDebugEmitters = false;                                                                                   // Debug: render emitters as cubes.
    bool renderDebugEnvironmentProbes = false;                                                                          // Debug: render env probes as sprites.
    bool renderDebugForcefields = false;                                                                                // Debug: render invisible walls (clip brushes) as cubes.
    bool renderDebugCameras = false;                                                                                    // Debug: render cameras as sprites.
    bool renderDebugColliders = false;                                                                                  // Debug: render colliders with solid color.
    bool renderDebugSprings = false;                                                                                    // Debug: render force springs as icons.
    bool renderDebugGridHelper = false;                                                                                 // Debug: render a grid over the world, to help with alignment.

    bool cullFreezeCamera = false;                                                                                      // Stop updating culling, to allow you to look around with the current culled frustum.
    bool cullAdvanced = true;                                                                                           // Use advanced culling algorithm.
    bool cullDebug = false;                                                                                             // Debug: Render things that have been culled with a solid color.
    bool cullOcclusion = false;                                                                                         // Enable occlusion culling.

    bool variableShadingClassification = false;                                                                         // Enable variable-rate-shading classification systems.
    bool variableShadingDebug = false;                                                                                  // Debug: variable-rate-shading classification dumped to a buffer that the Editor can read.

    bool temporalAA = false;                                                                                            // Enable temporal anti-aliasing.
    bool temporalAADebug = false;                                                                                       // Debug: output temporal anti-aliasing data to a buffer that can be displayed in the Editor.

    size_t rtBounces = 3;                                                                                               // Maximum number of bounces allowed for a single ray in Ray Tracing.
    bool rtDebug = false;                                                                                               // Debug: output useful ray-tracing related data for the Editor.
    bool rtShadows = false;                                                                                             // Render shadows using ray-tracing - specifically, Path Query. This isn't very well supported, so disabled by default.

    bool tesselation = true;                                                                                            // Enable tesselation shaders.
    bool disableAlbedo = false;                                                                                         // Disable the use of albedo maps in shaders.
    bool forceDiffuse = false;                                                                                          // Force a universal diffuse lighting on objects. Basically, fullbright.

    bool screenspaceShadows = false;                                                                                    // Use fast screen-space reflections. Less accurate, but can be visually appeasing.
    bool useSurfelGI = false;                                                                                           // Enable Surfels for Global Illumination. Fast and accurate.
    bool surfelDebug = false;                                                                                           // Debug: Output lots of debug buffers for Surfel illumination that the Editor can consume.
    bool useDynamicDiffuseGI = false;                                                                                   // Enable Dynamic Diffuse Global Illumination using path tracing.
    bool ddgiDebug = false;                                                                                             // Debug: Output lots of debug buffers for DDGI for the Editor to consume.
    size_t ddgiRays = 256;                                                                                              // The maximum number of rays per pixel for DDGI. Higher is usually more accurate (and softer overall), but too low creates a very grainy appearance. Prefer between 200 and 800 for a balance between quality and performance.
    float ddgiBlendSpeed = 0.1;                                                                                         // How fast to change colors utilizing new data from DDGI rays. Higher values mean the color becomes accurate faster, but rapidly changing colors can be disorienting and distracting, so a slow blend is usually better. Should be 0.1 for the least distracting experience.
    bool useVXGI = false;                                                                                               // Enable Voxel Global Illumination. Only supported on NVIDIA GPUs. Rather slow to coalesce to the final color, but accurate enough until it does.
    bool useVXGIReflections = true;                                                                                     // Use VXGI for reflections, if VXGI is enabled.
    bool vxgiDebug = false;                                                                                             // Debug: Output lots of debug bufers for VXGI for the Editor to consume.
    int vxgiDebugClip = 0;                                                                                              // A maximum limit on how far away from something must be to be considered for VXGI. 0 is no limit.

    float giBoost = 1;                                                                                                  // A color boost on Global Illumination values.

    std::atomic<size_t> erroredShaders { 0 };                                                                          // The number of shaders that errored while loading.
    std::atomic<size_t> missingShaders { 0 };                                                                          // The number of shaders that were requested to load but could not be found.

    std::vector<Renderer::CustomShader> customShaders;                                                                  // The custom shaders that are currently loaded by the renderer, and accessible by level rendering.

    Texture shadowMaps;                                                                                                 // A global shadow map atlas.
    Texture transparentShadowMaps;                                                                                      // A global shadow map atlas specifically for objects with transparency.
    int maxShadowRes2 = 1024;                                                                                           // The size of the shadow map, when accessed from a 2D view.
    int maxShadowResC = 256;                                                                                            // The size of the shadow map, when accessed from a Cube view.

    /* General note: variables ending in D in the following block relate to depth buffers. */
    std::vector<std::pair<DirectX::XMFLOAT4X4, DirectX::XMFLOAT4>> renderCubes, renderCubesD;                           // Cuboids to render.
    std::vector<std::pair<Sphere, DirectX::XMFLOAT4>> renderSpheres, renderSpheresD;                                    // Spheres to render.
    std::vector<std::pair<Capsule, DirectX::XMFLOAT4>> renderCapsules, renderCapsulesD;                                 // Capsules to render.
    std::vector<Renderer::LineRenderData> renderLines, renderLinesD;                                                    // Lines (3D and 2D) to render.
    std::vector<Renderer::PointRenderData> renderPoints, renderPointsD;                                                 // Points to render.
                                                                                                                        // Triangles to render, solid and wireframe.
    std::vector<Renderer::TriangleRenderData> renderTrianglesSolid, renderTrainglesWireframe, renderTrianglesSolidD, renderTrianglesWireframeD;

    std::vector<uint8_t> debugTexts;                                                                                    // A buffer of texts used to send data to the console from the renderer.

    SH::SpinLock mipperLock;                                                                                  // A lock used by the dynamic mipmap generator.
    std::vector<std::pair<Texture, bool>> mipperList;                                                                   // A list of textures for the dynamic mipmap generator to process. pair<Texture, whether it has been processed yet>
    std::vector<std::pair<Texture, Texture>> decompressorList;                                                          // A list of textures for the decompressor to work on. pair<compressed texture, decompressed texture>

    GPUBuffer luminanceBuffer;                                                                                          // A buffer for dynamic luminance compensation. Required by some shaders to exist. NVIDIA 10xx GPUs will error without such.

    SH::Jobs::ExecutionContext pipelineJobContext[ENUMSIZE(defs::RenderPass)];                                              // Job contexts for processing PSOs.

    PipelineState PSOOcclusion;                                                                                         // PSO for Occlusion Queries
    PipelineState PSOBillboard[ENUMSIZE(defs::RenderPass)];                                                                 // PSO for billboard rendering, per render pass
    PipelineState PSOBillboardWire;                                                                                     // PSO for billboard rendering of wires and lines.
    PipelineState PSOGatherBillboard;                                                                                   // PSO for capturing billboard-rendered pixels into a buffer
    PipelineState PSOLightVisual[ENUMSIZE(defs::LightType)];                                                                // PSO for visualizing light-affected pixels into a buffer
    PipelineState PSOLightVolumetric[ENUMSIZE(defs::LightType)];                                                            // PSO for rendering volumetric lights
    PipelineState PSOLightmap;                                                                                          // PSO for rendering light maps
    PipelineState PSOLensFlare;                                                                                         // PSO for rendering lens flares
    PipelineState PSODownsampleDepth;                                                                                   // PSO for downsampling depth buffers
    PipelineState PSOUpsample;                                                                                          // PSO for bilateral upsampling
    PipelineState PSOUpsampleClouds;                                                                                    // PSO for upsampling volumetric clouds
    PipelineState PSOOutline;                                                                                           // PSO for rendering outlines of objects
    PipelineState PSOSky[ENUMSIZE(defs::SkyRenderType)];                                                                    // PSO for sky rendering, per type
    PipelineState PSODebug[ENUMSIZE(defs::DebugRenderType)];                                                                // PSO for debug rendering, per mode
    PipelineState PSOWire;                                                                                              // PSO for wire (thin strips of pixels) rendering
    PipelineState PSOWireTess;                                                                                          // PSO for wire (thin strips of pixels) rendering, with tesselation

    RaytracingPipeline PSORTReflect;                                                                                    // PSO for ray-traced reflections


    /**
     * An instance of a mesh, with associated distance to the camera.
     * Can be sorted back-to-front or front-to-back depending on need.
     */
    struct OrderedRenderInstance {
        uint32_t mesh, instance;
        uint16_t distance, camera;
        uint32_t sort;

        /**
         * Create a new Ordered Render Instance.
         * The instance is immediately able to be sorted.
         * @param meshIdx     the index of the mesh to render
         * @param instanceIdx the specific instance of the mesh to render
         * @param distance    the distance of the center of the mesh from the camera.
         * @param sortBits    internal bits used to sort
         * @param cameraMask  which specific camera to reference - default to the main viewport camera.
         */
        OrderedRenderInstance(uint32_t meshIdx, uint32_t instanceIdx, float distance, uint32_t sortBits, uint16_t cameraMask = 0xFFFF) {
          mesh = meshIdx;
          instance = instanceIdx;
          this->distance = DirectX::PackedVector::XMConvertFloatToHalf(distance);
          sort = sortBits;
          camera = cameraMask;
        }

        inline float GetDistance() const {
            return DirectX::PackedVector::XMConvertHalfToFloat(DirectX::PackedVector::HALF(distance));
        }

        constexpr uint32_t GetMesh() const { return mesh; }
        constexpr uint32_t GetInstance() const { return instance; }

        /**
         * Opaque sorting
         * Front to back, with the rearmost objects ordered at the end of the list.
         * @param other another Ordered Render Instance to compare against.
         * @return whether this object should be in front of the other.
         */
        constexpr bool operator<(const OrderedRenderInstance& other) const {
            union Sort {
                // sort > mesh > distance priority order.
                struct __attribute__((packed)) { uint16_t distance; uint16_t mesh; uint32_t sort; } parts;
                size_t data;
            };

            Sort a { .parts = { .distance = this->distance, .mesh = static_cast<uint16_t>(this->mesh), .sort = this->sort }};
            Sort b { .parts = { .distance = other.distance, .mesh = static_cast<uint16_t>(other.mesh), .sort = other.sort }};
            return a.data < b.data;
        }

        /**
         * Transparent sorting
         * Back to front, with the rearmost object at the start of the list.
         * @param other another Ordered Render Instance to compare against.
         * @return whether this object should be behind the other.
         */
        constexpr bool operator>(const OrderedRenderInstance& other) const {
            union Sort {
                // distance > sort > mesh priority order.
                struct __attribute__((packed)) { uint16_t mesh; uint32_t sort; uint16_t distance; } parts;
                size_t data;
            };

            const Sort a { .parts = { .mesh = static_cast<uint16_t>(this->mesh), .sort = this->sort, .distance = this->distance } };
            const Sort b { .parts = { .mesh = static_cast<uint16_t>(other.mesh), .sort = other.sort, .distance = other.distance } };
            return a.data > b.data;
        }
    };

    /**
     * A list of Ordered Render Instances that would be collated into a draw call.
     */
    struct DrawBatch {
        std::vector<OrderedRenderInstance> instances;

        inline void Reset() { instances.clear(); }
        inline void Add(uint32_t mesh, uint32_t instance, float dist, uint32_t sort, uint16_t cameraMask = 0xFFFF) {
            instances.emplace_back(OrderedRenderInstance { mesh, instance, dist, sort, cameraMask });
        }
        inline void SortTransparent() {
            std::sort(instances.begin(), instances.end(), std::greater<OrderedRenderInstance>());
        }
        inline void SortOpaque() {
            std::sort(instances.begin(), instances.end(), std::less<OrderedRenderInstance>());
        }

        inline bool Empty() const { return instances.empty(); }
        inline size_t Size() const { return instances.size(); }
    };

    /**
     * A simple way to encode the enumerations of render passes based on the many factors.
     * Use as an index into the array of pipeline state objects.
     */
    union RenderVariants {
        struct __attribute__((packed)) { uint8_t pass : 4; uint8_t shader; uint8_t blend : 4; uint8_t cull : 2; uint8_t tesselation : 1; uint8_t alpha : 1; uint32_t sample : 4; } parts;
        uint32_t data;
    };

    std::unordered_map<uint32_t, PipelineState> PSOByVariant[ENUMSIZE(defs::RenderPass)][ENUMSIZE(defs::MaterialShaderType)]; // TODO: Material Component types
    inline PipelineState* GetPipelineForVariants(RenderVariants var) {
        return &PSOByVariant[var.parts.pass][var.parts.shader][var.data];
    }

    defs::ShaderType VertexShaderFor(defs::RenderPass pass, bool tesselation, bool alpha, bool transparent) {
        switch (pass) {
            case defs::RenderPass::MAIN: return tesselation ? defs::ShaderType::V_OBJECT_COMMON_TESS : defs::ShaderType::V_OBJECT_COMMON;
            case defs::RenderPass::PRE:
            case defs::RenderPass::PRE_DEPTH:
                return tesselation
                    ? alpha ? defs::ShaderType::V_OBJECT_PRE_ALPHA_TESS : defs::ShaderType::V_OBJECT_PRE_TESS
                    : alpha ? defs::ShaderType::V_OBJECT_PRE_ALPHA : defs::ShaderType::V_OBJECT_PRE;
            case defs::RenderPass::ENVMAPPING:
                return defs::ShaderType::V_ENVMAP;
            case defs::RenderPass::SHADOW:
                return transparent ? defs::ShaderType::V_SHADOW_TRANSPARENT :
                    alpha ? defs::ShaderType::V_SHADOW_ALPHA : defs::ShaderType::V_SHADOW;
            case defs::RenderPass::VOXEL: return defs::ShaderType::V_VOXELIZER;
            case defs::RenderPass::SKY_BLOCK: return defs::ShaderType::V_SHADOW;
        }
    }

    defs::ShaderType GeometryShaderFor(defs::RenderPass pass, bool alpha, bool transparent) {
        switch (pass) {
            case defs::RenderPass::VOXEL: return defs::ShaderType::G_VOXELIZER;
            case defs::RenderPass::PRE: break;
            case defs::RenderPass::ENVMAPPING:
                if (iface->CheckCapability(GraphicsDeviceCapability::RT_VIEWPORT_WITHOUT_GEOMETRY_SHADER)) break;
                return defs::ShaderType::G_ENVMAP_EMULATE;
            case defs::RenderPass::SHADOW:
                if (iface->CheckCapability(GraphicsDeviceCapability::RT_VIEWPORT_WITHOUT_GEOMETRY_SHADER)) break;
                return transparent ? defs::ShaderType::G_SHADOW_TRANSPARENT_EMULATE : alpha ? defs::ShaderType::G_SHADOW_ALPHA_EMULATE : defs::ShaderType::G_SHADOW_EMULATE;
        }
    }

    defs::ShaderType HullShaderFor(defs::RenderPass pass, bool tesselation, bool alpha) {
        if (tesselation) {
            if (pass == defs::RenderPass::MAIN)
                return defs::ShaderType::H_OBJECT;
            if (pass == defs::RenderPass::PRE || pass == defs::RenderPass::PRE_DEPTH)
                return alpha ? defs::ShaderType::H_OBJECT_PRE_ALPHA : defs::ShaderType::H_OBJECT_PRE;
        }
        return defs::ShaderType::SIZE;
    }

    defs::ShaderType DomainShaderFor(defs::RenderPass pass, bool tesselation, bool alpha) {
        if (tesselation) {
            if (pass == defs::RenderPass::MAIN)
                return defs::ShaderType::D_OBJECT;
            if (pass == defs::RenderPass::PRE || pass == defs::RenderPass::PRE_DEPTH)
                return alpha ? defs::ShaderType::D_OBJECT_PRE_ALPHA : defs::ShaderType::D_OBJECT_PRE;
        }
        return defs::ShaderType::SIZE;
    }

    defs::ShaderType PixelShaderFor(defs::RenderPass pass, bool alpha, bool transparent, int shader) {
        switch (pass) {
            case defs::RenderPass::MAIN: return static_cast<defs::ShaderType>(static_cast<int>(transparent ? defs::ShaderType::P_OBJECT_TRANSPARENT_PERMUTE_BEGIN : defs::ShaderType::P_OBJECT_PERMUTE_BEGIN) + shader);
            case defs::RenderPass::PRE: return alpha ? defs::ShaderType::P_OBJECT_PRE_ALPHA : defs::ShaderType::P_OBJECT_PRE;
            case defs::RenderPass::PRE_DEPTH: return alpha ? defs::ShaderType::P_OBJECT_PRE_DEPTH_ALPHA : defs::ShaderType::P_OBJECT_PRE_DEPTH;
            case defs::RenderPass::ENVMAPPING: return defs::ShaderType::P_ENVMAP;
            case defs::RenderPass::SHADOW: return transparent ? defs::ShaderType::P_SHADOW_TRANSPARENT /* TODO: Water Material Shader */ : alpha ? defs::ShaderType::P_SHADOW_ALPHA : defs::ShaderType::SIZE;
        }
    }
}

namespace rx {

    size_t Renderer::GetShaderErrorCount() {
        return internal::erroredShaders;
    }

    size_t Renderer::GetShaderMissingCount() {
        return internal::missingShaders;
    }

    const Sampler *Renderer::GetSampler(rx::defs::SamplerType ID) {
        return &rx::internal::samplers[static_cast<uint32_t>(ID)];
    }
    const Shader *Renderer::GetShader(rx::defs::ShaderType ID) {
        return &rx::internal::shaders[static_cast<uint32_t>(ID)];
    }
    const InputLayout *Renderer::GetInputLayout(rx::defs::InputLayout ID) {
        return &rx::internal::layouts[static_cast<uint32_t>(ID)];
    }
    const RasterizerState *Renderer::GetRasterizerState(rx::defs::RasterizerState ID) {
        return &rx::internal::rasters[static_cast<uint32_t>(ID)];
    }
    const DepthStencilState *Renderer::GetDepthStencilState(rx::defs::DepthStencilState ID) {
        return &rx::internal::stencils[static_cast<uint32_t>(ID)];
    }
    const BlendState *Renderer::GetBlendState(rx::defs::BlendModes ID) {
        return &rx::internal::blends[static_cast<uint32_t>(ID)];
    }
    const GPUBuffer *Renderer::GetBuffer(rx::defs::BufferType ID) {
        return &rx::internal::buffers[static_cast<uint32_t>(ID)];
    }
    const Texture *Renderer::GetTexture(rx::defs::TextureType ID) {
        return &rx::internal::texs[static_cast<uint32_t>(ID)];
    }

    const std::string& Renderer::GetShaderPath() {
        return internal::shaderBinaryPath.c_str();
    }

    const std::string& Renderer::GetShaderSourcePath() {
        return internal::shaderSourcePath.c_str();
    }

    const std::vector<Renderer::CustomShader>& Renderer::GetCustomShaders() {
        return internal::customShaders;
    }

    int Renderer::RegisterCustomShader(const CustomShader& shader) {
        static std::mutex lock;
        std::scoped_lock locker(lock);
        internal::customShaders.push_back(shader);
        return internal::customShaders.size() - 1;
    }

    bool Renderer::LoadShader(ShaderStage stage, Shader& out, const std::string& filename, ShaderModel model, std::vector<std::string> permute_defines) {

        SH::Path shaderBinaryFile = GetShaderPath() + filename;

        rx::shader::RegisterShader(shaderBinaryFile);

        if (!rx::shader::IsMetadataOutdated(shaderBinaryFile)) {
            rx::shader::CompilerInput in {
                .format = internal::iface->GetShaderFormat(),
                .stage = stage,
                .model = model,
                .defines = permute_defines
            };

            SH::Path source = GetShaderSourcePath();
            source.makeAbsolute();

            in.includes.push_back(source.c_str());
            in.includes.push_back((source + SH::Path::getDirectory(filename)).c_str());
            in.sourceName = (source + filename).replaceExtension("hlsl").c_str();

            rx::shader::CompilerOutput output;
            rx::shader::Compile(in, output);

            if (out.IsValid()) {
                rx::shader::SaveData(shaderBinaryFile, output);

                if (!output.error.empty())
                    spdlog::warn(output.error);
                spdlog::trace("Shader Compiled: " + shaderBinaryFile.toString());
                return internal::iface->CreateShader(stage, output.data, output.dataLen, &out);
            } else {
                spdlog::error("Shader Compilation failed: " + shaderBinaryFile.toString());
                internal::erroredShaders.fetch_add(1);
            }
        }

        if (internal::iface != nullptr) {
            std::vector<uint8_t> buf;
            if (internal::fs->readSync(shaderBinaryFile, buf)) {
                bool success = internal::iface->CreateShader(stage, buf.data(), buf.size(), &out);
                if (success)
                    internal::iface->SetName(&out, shaderBinaryFile.c_str());
            } else {
                internal::missingShaders.fetch_add(1);
            }
        }

        return false;
    }

    bool Renderer::IsStateInitializing() {
        for (uint32_t renderPass = 0; renderPass < static_cast<uint32_t>(defs::RenderPass::SIZE); renderPass++)
            if (SH::Jobs::IsWorking(internal::pipelineJobContext[renderPass]))
                return true;
    }

    void SetupShaders() {

    }

    void SetupBuffers() {
        GPUBufferMeta meta {
            .size = sizeof(FrameConstants),
            .usage = BufferUsage::STAGING,
            .binding = BindFlag::CONSTANT_BUFFER
        };
        internal::iface->CreateBuffer(&meta, nullptr, &internal::buffers[static_cast<uint32_t>(defs::BufferType::FRAME)]);
        internal::iface->SetName(&internal::buffers[static_cast<uint32_t>(defs::BufferType::FRAME)], "Frame Constants buffer");
    }

    void SetupStates() {

    }

    void Renderer::Initialize() {
        SH::Timer timer;

        SetupStates();
        SetupBuffers();
        SetupShaders();

        spdlog::info("rx::Renderer initialized, " + std::to_string(timer.elapsedMillis()) + "ms");
        internal::ready.store(true);
    }



}