#pragma once

#include <cstdint>
#include <SDL_video.h>
#include "GraphicsDefine.h"
#include <functional>
#include <cassert>
#include <cstring>

/**
 * This file contains the "interface" for the Renderer to interact with the GPU.
 * It is essentially an abstraction over the Vulkan and DX12 APIs.
 * This should be the base class of a hotswappable renderer module.
 */

namespace rx {

    static constexpr uint32_t BIND_CONSTANTS = 14;
    static constexpr uint32_t BIND_SHADERS = 16;
    static constexpr uint32_t BIND_UNIFORMS = 16;
    static constexpr uint32_t BIND_SAMPLERS = 8;
    struct DescriptorBinds {
      GPUBuffer constants[BIND_CONSTANTS];
      size_t constantOffsets[BIND_CONSTANTS] = {};
      GPUResource shaders[BIND_SHADERS];
      int shaderIndex[BIND_SHADERS] = {};
      GPUResource uniforms[BIND_UNIFORMS];
      int uniformIndex[BIND_UNIFORMS];
      Sampler samplers[BIND_SAMPLERS];
    };

    /**
     * A command list owned by a single thread.
     * Multiple threads may write to their own command list at the same time.
     * Multiple threads may not write to the same command list at the same time.
     *
     * Start with rx::BeginCommands(). Submit ALL COMMAND LISTS with rx:SubmitCommands().
     */
    struct ThreadCommands {
        void* internal = nullptr;
        constexpr bool isValid() const { return internal != nullptr; }
    };

    /**
     * Queues are highly specialized to a specific usecase.
     * Using the right queue can make all the difference for performance.
     */
    enum QueueType {
        GRAPHICS,
        COMPUTE,
        COPY,   // PRESENTATION, in Vulkan.
        VIDEO_DECODE,
        COUNT
    };

    /**
     * The interface to the graphics device.
     * The superclasses of this (GraphicsVulkan and GraphicsDX12) will implement all of these functions via their specific methods.
     * Everything that will be needed in the engine is exposed through this interface.
     *
     * Implement (consume) the Interface in an instance of RenderImpl.
     */
    class Interface {
    protected:
        static constexpr uint32_t FRAMEBUFFERS = 2; // Double buffering by default!
        size_t frameCount = 0; // Increments by 1 every time a game frame is rendered.
        Validation validation = Validation::DISABLED; // Get debug errors from the GPU and CPU
        GraphicsDeviceCapability capabilities = GraphicsDeviceCapability::NONE; // See what the GPU is capable of.
        size_t shaderNameSize = 0;
        size_t topLevelAccelerationInstanceSize = 0;
        uint32_t variableRateShadingTileSize = 0;
        size_t timestampFrequency = 0; // How often to ask the GPU "how long has this task been running", in number per second.
        size_t videoDecodeBitstreamAlignment = 0;
        uint32_t vendorID = 0;
        uint32_t deviceID = 0;
        std::string deviceName;
        std::string driverDescription;
        RenderDeviceType type = RenderDeviceType::OTHER;

    public:
        virtual ~Interface() = default;

        // Supports re-creating a swapchain in-place with a changed meta. The window may be null if sc is not null.
        virtual bool CreateSwapchain(const SwapchainMeta* meta, SDL_Window* window, SwapChain* sc) const = 0;
        // Start a buffer on the GPU. Do not read from dest.
        virtual bool CreateBuffer(const GPUBufferMeta* meta, const std::function<void(void* dest)>& callback, GPUBuffer* buffer) const = 0;
        // Start a texture, with optional initial data.
        virtual bool CreateTexture(const TextureMeta* meta, const SubresourceMeta* subresource, Texture* tex) const = 0;
        // Start a shader from bytecode. Does not accept source, in glsl or hlsl.
        virtual bool CreateShader(ShaderStage stage, const void* code, size_t size, Shader* shader) const = 0;
        // Start a sampler for a generic sample type.
        virtual bool CreateSampler(const SamplerMeta* meta, Sampler* sampler) const = 0;
        // Start a query heap, of query objects to be sent to the GPU.
        virtual bool CreateQueryHeap(const GPUQueryHeapMeta* meta, GPUQueryHeap* heap) const = 0;
        // Start a PSO. If rpm is passed, PSO will be created immediately, but limited to just that RPM.
        virtual bool CreatePipelineState(const PipelineStateMeta* meta, PipelineState* pso, const RenderPassMeta* rpm = nullptr) const = 0;
        // Start a ray tracing acceleration structure. Rather useless if the GPU does not support accelerated raytracing.
        virtual bool CreateRayTracingAccelerationStructure(const RaytracingAccelerationMeta* meta, RaytracingAcceleration* accel) const { return false; };
        // Start a ray tracing pso.
        virtual bool CreateRayTracingPipleineState(const RaytracingPipelineMeta* meta, RaytracingPipeline* pso) const { return false; }
        // Start a video decoder that runs on hardware.
        virtual bool CreateVideoDecoder(const VideoMeta* meta, VideoDecoder* decoder) const { return false; }

        // Start a subresource reference for a texture.
        virtual bool CreateSubresource(Texture* tex, SubresourceMeta meta, uint32_t firstSlice, uint32_t sliceCount, uint32_t firstMip, uint32_t mipCount, const ImageFormat* formatChange = nullptr, const ImageAspect* aspect = nullptr, const Swizzle* swizzle = nullptr) = 0;
        // Start a subresource reference to a buffer.
        virtual bool CreateSubresource(GPUBuffer* buf, SubresourceMeta meta, size_t offset, size_t size = ~0u, const ImageFormat* formatChange = nullptr, uint32_t* strideChange = nullptr) const = 0;

        // Fetch the descriptor index for a (sub)resource in the current active shader. TODO: is this necessary with bindless?
        virtual int GetDescriptorIdx(const GPUResource* resource, ImageViewType sub, int subIdx = -1) const = 0;
        // Fetch the descriptor info for a sampler in the current active shader. TODO: is this necessary with bindless?
        virtual int GetDescriptorIdx(const Sampler* sampler) const = 0;

        // Write a new value into the shading rate data in GPU memory.
        virtual int WriteShadingRateValue(ShadingRate rate, void* data) const {};
        // Write a new value into the Ray Tracing Acceleration Top Level structure.
        virtual int WriteTLRTAcceleration(const RaytracingAccelerationMeta::TopLevel::Instance* instance, void* dest) const {}
        // Set a new shader for a specific hit group entry.
        virtual int WriteShaderIdentifier(const RaytracingAccelerationMeta* meta, uint32_t idx, void* data) const {}

        // For debugging: assign the resource a name ON-GPU
        virtual void SetName(GPUResource* resource, const char* name) const {}
        virtual void setName(Shader* shader, const char* name) const {}

        // Start a new command list for the current thread and begin listening.
        virtual ThreadCommands BeginCommands(QueueType queue = QueueType::GRAPHICS) = 0;

        // Submit all command lists that were created from the current device, across all threads.
        virtual void SubmitCommands() = 0;

        // Wait for all submitted work to complete.
        virtual void WaitForGPU() const = 0;

        // Clear pipleline cache - usually when reloading shaders!
        virtual void ClearPipelineCache() = 0;

        // Fetch how many pipelines are active.
        // Should be pretty low.
        virtual size_t GetPipelineCount() const = 0;

        // Return the number of elapsed submits.
        // Does not represent the number of actual "primary" frames submitted, more the total number of render operations.
        constexpr size_t GetElapsedFrames() const { return frameCount; }

        // Check whether the device supports at least one of the given set of capabilities.
        // Ideally, check one at a time.
        constexpr bool CheckCapability(GraphicsDeviceCapability cap) const { return has_flag(capabilities, cap); }

        // Get the number of framebuffers in use.
        static constexpr uint32_t GetBufferCount() { return FRAMEBUFFERS; }
        // Get the current "primary" framebuffer.
        constexpr uint32_t GetBufferIndex() const { return GetElapsedFrames() % GetBufferCount(); }

        // Get whether debug features are enabled on the GPU.
        constexpr bool isDebugEnabled() const { return validation != Validation::DISABLED; }

        // Get the maximum size of a shader identifier.
        constexpr size_t GetShaderIdentifierSize() const { return shaderNameSize; }
        // Get the size of an instance of a Top Level Ray Tracing Acceleration structure.
        constexpr size_t GetTLRTAccelerationInstanceSize() const { return topLevelAccelerationInstanceSize; }
        // Get the tile size of the variable rate shading buffer.
        constexpr uint32_t GetVariableRateShadingTileSize() const { return variableRateShadingTileSize; }
        // Get the frequency of timestamp query requests to the GPU.
        constexpr size_t GetTimestampFrequency() const { return timestampFrequency; }
        // Get the alignment of a bitstream for a Video Decoder.
        constexpr size_t GetVideoDecodeBitstreamAlignmentSize() const { return videoDecodeBitstreamAlignment; }

        // Get the ID of the vendor of the active GPU.
        constexpr uint32_t GetVendorID() const { return vendorID; }
        // Get the ID of the device (model) of the active GPU.
        constexpr uint32_t GetDeviceID() const { return deviceID; }

        // Get the name of the GPU in readable words.
        constexpr const std::string& GetDeviceName() const { return deviceName; }
        // Get a description from the driver of the device.
        constexpr const std::string& GetDriverDescription() const { return driverDescription; }
        // Get the type of the active GPU.
        constexpr RenderDeviceType GetDeviceType() const { return type; }

        // Get the byte format that the current device expects its shaders in.
        virtual ShaderFormat GetShaderFormat() const = 0;

        // Get the Texture resource that represents the current backbuffer (renderable)
        virtual Texture GetBackbuffer(const SwapChain* sc) const = 0;
        // Get the color space of the current swapchain
        virtual ColorSpace GetColorspace(const SwapChain* sc) const = 0;
        // Check whether the Swapchain could support HDR.
        virtual bool SwapchainSupportsHDR(const SwapChain* sc) const = 0;

        // Get the minimum offset alignment for creating subresources in buffers.
        virtual size_t GetMinOffsetAlignment(const GPUBufferMeta* meta) const = 0;

        struct MemoryUsage {
            size_t budget = 0;
            size_t usage = 0;
        };
        // Get the total memory available, and the total memory used.
        virtual MemoryUsage GetMemoryUsage() const = 0;

        // Get the max amount of viewports that can be bound at a time.
        virtual uint32_t GetMaxViewports() const = 0;

        // Batch update of sparse resources
        virtual void SparesUpdate(QueueType queue, const SparseUpdateCommand* commamds, uint32_t commandCount) {}

        /**
         * Command list stuff
         */

        // Wait for the other command list to complete.
        virtual void WaitForCommands(ThreadCommands cmd, ThreadCommands waitFor) = 0;
        // Begin a render pass on the swapchain.
        virtual void BeginRenderPass(const SwapChain* sc, ThreadCommands cmd) = 0;
        // Begin a render pass on the given image.
        virtual void BeginRenderPass(const RenderPassImage* imgs, uint32_t imageCount, ThreadCommands cmd) = 0;
        // End a render pass
        virtual void EndRenderPass(ThreadCommands cmd) = 0;
        // Bind rectangles to the scissor buffer.
        virtual void BindScissorRectangles(const Rect* rects, uint32_t rectCount, ThreadCommands cmd) = 0;
        // Bind viewports to be rendered
        virtual void BindViewports(const Viewport* viewports, uint32_t vpCount, ThreadCommands cmd) = 0;
        // Bind a resource to a shader slot
        virtual void BindResource(const GPUResource* res, uint32_t slot, ThreadCommands cmd, int subResource = -1) = 0;
        // Bind multiple resources to a slot list in order.
        virtual void BindResources(const GPUResource* const* resources, uint32_t startSlot, uint32_t count, ThreadCommands cmd) = 0;
        // Bind an Unordered Access View to a shader slot
        virtual void BindUAV(const GPUResource* res, uint32_t slot, ThreadCommands cmd, int subResource = -1) = 0;
        // Bind multiple UAVs to a slot list in order.
        virtual void BindUAVs(const GPUResource* const* resource, uint32_t startSlot, uint32_t count, ThreadCommands cmd) = 0;
        // Bind a sampler to a shader slot.
        virtual void BindSampler(const Sampler* sampler, uint32_t slot, ThreadCommands cmd) = 0;
        // Bind a constant buffer to a shader slot.
        virtual void BindConstants(const GPUBuffer* buffer, uint32_t slot, ThreadCommands cmd, size_t offset = 0ull) = 0;
        // Bind vertex buffers to a shader slot list.
        virtual void BindVertexes(const GPUBuffer* const* buffers, uint32_t slot, uint32_t count, const uint32_t* offsets, ThreadCommands cmd) = 0;
        // Bind index buffer to a shader slot.
        virtual void BindIndex(const GPUBuffer* buffer, const IndexBufferFormat fmt, size_t offset, ThreadCommands cmd) = 0;
        // Bind a reference to the stencil buffer.
        virtual void BindStencil(uint32_t value, ThreadCommands cmd) = 0;
        // Bind a new blend factor.
        virtual void BindBlend(float r, float g, float b, float a, ThreadCommands cmd) = 0;
        // Bind a new shading rate.
        virtual void BindShadingRate(ShadingRate rate, ThreadCommands cmd) = 0;
        // Bind a new PSO
        virtual void BindPSO(const PipelineState* pso, ThreadCommands cmd) = 0;
        // Bind a compute shader for execution
        virtual void BindCompute(const Shader* cs, ThreadCommands cmd) = 0;
        // Bind new bounds for the depth test.
        virtual void BindDepthBounds(float min, float max, ThreadCommands cmd) = 0;

        // Draw vertices to the screen from the active vertex buffer. Draws vertexCount vertices, starting from vertex index firstVertex in the buffer.
        virtual void Draw(uint32_t vertexCount, uint32_t firstVertex, ThreadCommands cmd) = 0;
        // Draw indexed vertexes to the screen, reading from active vertex buffer and active index buffer.
        virtual void DrawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t firstVertex, ThreadCommands cmd) = 0;
        // Run Draw per instance in the buffer.
        virtual void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance, ThreadCommands cmd) = 0;
        // Run DrawIndexed per instance in the buffer.
        virtual void DrawIndexedInstanced(uint32_t indexCounts, uint32_t instanceCount, uint32_t firstIndex, int32_t firstVertex, uint32_t firstInstance, ThreadCommands cmd) = 0;
        // Indirectly draw (let the shader figure out the arguments) an indexed object
        virtual void DrawIndexedIndirect(const GPUBuffer* args, size_t offset, ThreadCommands cmd) = 0;
        // Indirectly draw an instanced indexed mesh
        virtual void DrawIndexedInstanceIndirect(const GPUBuffer* args, size_t offset, ThreadCommands cmd) = 0;
        // Indirectly draw a configurable amount of indexed meshes.
        virtual void DrawIndexedIndirectCount(const GPUBuffer* args, size_t offset, const GPUBuffer* count, size_t countOffset, uint32_t maxCount, ThreadCommands cmd) = 0;
        // Indirectly draw a configurable amount of instanced indirect meshes.
        virtual void DrawIndexedInstancedIndirectCount(const GPUBuffer* args, size_t offset, const GPUBuffer* count, size_t countOffset, uint32_t maxCount, ThreadCommands cmd) = 0;

        // Dispatch a compute shader in parallel.
        virtual void Dispatch(uint32_t tCountX, uint32_t tCountY, uint32_t tCountZ, ThreadCommands cmd) = 0;
        // Dispatch a compute shader with its own set of args.
        virtual void DispatchIndirect(const GPUBuffer* args, size_t offset, ThreadCommands cmd) = 0;
        // Dispatch a compute shader with the vertex and index buffers.
        virtual void DispatchMesh(uint32_t tCountX, uint32_t tCountY, uint32_t tCountZ, ThreadCommands cmd) {}
        // Dispatch a compute shader with vertex and index buffers passed indirectly.
        virtual void DispatchMeshIndirect(const GPUBuffer* args, size_t offset, ThreadCommands cmd) {}
        // Dispatch a compute shader with vertex and index buffers passed indirectly with a set number of instances
        virtual void DispatchMeshIndirectCount(const GPUBuffer* args, size_t offset, const GPUBuffer* count, size_t countOffset, uint32_t maxCount, ThreadCommands cmd) {}

        // Copy a resource from source to dest.
        virtual void CopyResource(const GPUResource* source, const GPUResource* dest, ThreadCommands cmd) = 0;
        // Copy a buffer from source to dest.
        virtual void CopyBuffer(const GPUBuffer* source, size_t sourceOffset, const GPUBuffer* dest, size_t destOffset, size_t size, ThreadCommands cmd) = 0;
        // Copy a texture from source to dest.
        virtual void CopyTexture(const Texture* source, const Texture* dest, uint32_t destX, uint32_t destY, uint32_t destZ, uint32_t destMip, uint32_t destSlice, uint32_t sourceMip, uint32_t sourceSlice, ThreadCommands cmd, const Box* sourceBox = nullptr, ImageAspect destAspect = ImageAspect::COLOR, ImageAspect sourceAspect = ImageAspect::COLOR) = 0;

        // Start a query
        virtual void QueryBegin(const GPUQueryHeap* heap, uint32_t idx, ThreadCommands cmd) = 0;
        // End a query
        virtual void QueryEnd(const GPUQueryHeap* heap, uint32_t idx, ThreadCommands cmd) = 0;
        // Fetch the results of the query(s)
        virtual void QueryFetch(const GPUQueryHeap* heap, uint32_t idx, uint32_t count, const GPUBuffer* dest, size_t offset, ThreadCommands cmd) = 0;
        // Reset an arbitrary number of queries
        virtual void QueryReset(const GPUQueryHeap* heap, uint32_t idx, uint32_t count, ThreadCommands cmd) {}

        // Barrier a state change for a buffer or texture.
        virtual void Barrier(const Barrier* barriers, uint32_t num, ThreadCommands cmd) = 0;

        // Update push constants
        virtual void PushConstants(const void* data, uint32_t size, ThreadCommands cmd, uint32_t offset = 0) = 0;

        // Start a predication
        virtual void PredicationBegin(const GPUBuffer* buffer, size_t offset, PredicationOp op, ThreadCommands cmd) {}
        // End a predication
        virtual void PredicationEnd(ThreadCommands cmd) {}

        // Clear an Unordered Access View resource
        virtual void ClearUAV(const GPUResource* resource, uint32_t value, ThreadCommands cmd) = 0;

        // Decode a single frame of video
        virtual void VideoDecode(const VideoDecoder* decoder, const VideoDecodeOperation* op, ThreadCommands cmd) = 0;

        // Build an acceleration structure for RT.
        virtual void BuildRTAcceleration(const RaytracingAcceleration* dest, ThreadCommands cmd) {}
        // Bind a RT pipeline
        virtual void BindRayTracingPipeline(const RaytracingPipeline* pso, ThreadCommands cmd) {}
        // Dispatch a compute shader to calculate the rays
        virtual void DispatchRays(const RayDispatch* dispatch, ThreadCommands cmd) {}

        // Begin an event in the profiler
        virtual void EventBegin(const char* name, ThreadCommands cmd) = 0;
        // End the most recent even in the profiler
        virtual void EventEnd(ThreadCommands cmd) = 0;
        // Set a marker in the profiler
        virtual void SetMarker(const char* name, ThreadCommands cmd) = 0;

        virtual RenderPassMeta GetRenderPassMeta(ThreadCommands cmd) = 0;

        /**
         * Helper methods
         */

        // Start a GPU Buffer with intitial data.
        bool CreateBufferSimple(const GPUBufferMeta* meta, const void* initialData, GPUBuffer* buffer) const {
            if (!initialData)
                return CreateBuffer(meta, nullptr, buffer);
            else
                return CreateBuffer(meta, [&](void* dest) { memcpy(dest, initialData, meta->size); }, buffer);
        }

        struct GPULinearAllocator {
            GPUBuffer buffer;
            size_t offset = 0;
            size_t alignment = 0;
            void Reset() {
                offset = 0;
            }
        };

        // Get the allocator to be used for the current frame.
        virtual GPULinearAllocator& GetAllocator(ThreadCommands cmd) = 0;

        struct GPUAllocation {
            void* data = nullptr;
            GPUBuffer buffer;
            size_t offset = 0;

            inline bool IsValid() const { return data != nullptr && buffer.isValid(); }
        };

        // Start a staging buffer that will allow us to write to the GPU and copy it into a different buffer.
        // Suballocates out of a larger "framewise" buffer.
        GPUAllocation CreateStagingBuffer(size_t size, ThreadCommands cmd) {
            GPUAllocation alloc;
            if (size == 0) return alloc;

            GPULinearAllocator& allocator = GetAllocator(cmd);

            const size_t freeSpace = allocator.buffer.meta.size - allocator.offset;
            if (size > freeSpace) {
                GPUBufferMeta meta {
                    .usage = BufferUsage::STAGING,
                    .binding = BindFlag::CONSTANT_BUFFER | BindFlag::VERTEX_BUFFER | BindFlag::INDEX_BUFFER | BindFlag::SHADER_RESOURCE,
                    .flags = ResourceFlags::BUFFER_RAW,
                };

                allocator.alignment = GetMinOffsetAlignment(&meta);
                meta.size = AlignTo((allocator.buffer.meta.size + size) * 2, allocator.alignment);

                CreateBuffer(&meta, nullptr, &allocator.buffer);
                SetName(&allocator.buffer, "Frame Allocator");
                allocator.offset = 0;
            }

            alloc.buffer = allocator.buffer;
            alloc.offset = allocator.offset;
            alloc.data = (void*)((size_t) allocator.buffer.mapped + allocator.offset);
            allocator.offset += AlignTo(size, allocator.alignment);

            assert(alloc.IsValid());
            return alloc;
        }

        // Update a Device buffer.
        void UpdateBuffer(const GPUBuffer* buffer, const void* data, ThreadCommands cmd, size_t size = ~0, size_t offset = 0) {
            if (buffer == nullptr || data == nullptr) return;

            size = std::min(buffer->meta.size, size);
            if (size == 0) return;

            GPUAllocation alloc = CreateStagingBuffer(size, cmd);
            memcpy(alloc.data, data, size);
            CopyBuffer(buffer, offset, &alloc.buffer, alloc.offset, size, cmd);
        }

        template<typename T>
        void BindDynamicConstants(const T& data, uint32_t slot, ThreadCommands cmd) {
            GPUAllocation allocation = CreateStagingBuffer(sizeof(T), cmd);
            std::memcpy(allocation.data, &data, sizeof(T));
            BindConstants(&allocation.buffer, slot, cmd, allocation.offset);
        }

    };

    // Get the current active render device
    inline Interface*& GetInterface() {
        static Interface* device = nullptr;
        return device;
    }

}