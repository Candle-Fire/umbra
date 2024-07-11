#pragma once

#include <renderer/Interface.h>
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include "shadow/util/Synchronization.h"
#include <atomic>
#include <deque>

namespace rx {
  class VulkanInterface : public Interface {
    friend struct CommandQueue;

  protected:
    bool debug = false;
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    std::vector<VkQueueFamilyProperties2> queueFamilies;
    std::vector<VkQueueFamilyVideoPropertiesKHR> queueFamiliesVideo;
    size_t graphicsFamily = VK_QUEUE_FAMILY_IGNORED;
    size_t computeFamily = VK_QUEUE_FAMILY_IGNORED;
    size_t copyFamily = VK_QUEUE_FAMILY_IGNORED;
    size_t videoFamily = VK_QUEUE_FAMILY_IGNORED;
    std::vector<size_t> families;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue computeQueue = VK_NULL_HANDLE;
    VkQueue copyQueue = VK_NULL_HANDLE;
    VkQueue videoQueue = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties2 deviceProps2 = {};
    VkPhysicalDeviceVulkan11Properties deviceProps11 = {};
    VkPhysicalDeviceVulkan12Properties deviceProps12 = {};
    VkPhysicalDeviceVulkan13Properties deviceProps13 = {};
    VkPhysicalDeviceSamplerFilterMinmaxProperties samplerMinmaxProps = {};
    VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProps = {};
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingProps = {};
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragmentShadingRateProps = {};
    VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProps = {};
    VkPhysicalDeviceMemoryProperties2 memoryProps = {};
    VkPhysicalDeviceDepthStencilResolveProperties depthStencilResolveProps = {};
    VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
    VkPhysicalDeviceVulkan11Features deviceFeatures11 = {};
    VkPhysicalDeviceVulkan12Features deviceFeatures12 = {};
    VkPhysicalDeviceVulkan13Features deviceFeatures13 = {};
    VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureFeatures = {};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingFeatures = {};
    VkPhysicalDeviceRayQueryFeaturesKHR rayTracingQueryFeatures = {};
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeatures = {};
    VkPhysicalDeviceConditionalRenderingFeaturesEXT conditionalRenderingFeatures = {};
    VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures = {};
    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {};

    VkVideoDecodeH264ProfileInfoKHR decodeH264Profile = {};
    VkVideoDecodeH264CapabilitiesKHR devoceH264Capabilities = {};

    struct VideoCapability {
      VkVideoProfileInfoKHR profile = {};
      VkVideoDecodeCapabilitiesKHR decode = {};
      VkVideoCapabilitiesKHR capabilities = {};
    };
    VideoCapability capabilityH264 = {};

    std::vector<VkDynamicState> psoDynamicState;
    VkPipelineDynamicStateCreateInfo dynamicStateCreate = {};
    VkPipelineDynamicStateCreateInfo dynamicStateMeshCreate = {};

    VkBuffer nullBuffer = VK_NULL_HANDLE;
    VmaAllocation nullAllocation = VK_NULL_HANDLE;
    VkBufferView nullBufferView = VK_NULL_HANDLE;
    VkSampler nullSampler = VK_NULL_HANDLE;
    VmaAllocation nullImageBuffer1 = VK_NULL_HANDLE;
    VmaAllocation nullImageBuffer2 = VK_NULL_HANDLE;
    VmaAllocation nullImageBuffer3 = VK_NULL_HANDLE;
    VkImage nullImage1 = VK_NULL_HANDLE;
    VkImage nullImage2 = VK_NULL_HANDLE;
    VkImage nullImage3 = VK_NULL_HANDLE;
    VkImageView nullImageView1 = VK_NULL_HANDLE;
    VkImageView nullImageView1A = VK_NULL_HANDLE;
    VkImageView nullImageView2 = VK_NULL_HANDLE;
    VkImageView nullImageView2A = VK_NULL_HANDLE;
    VkImageView nullImageViewC = VK_NULL_HANDLE;
    VkImageView nullImageViewCA = VK_NULL_HANDLE;
    VkImageView nullImageView3 = VK_NULL_HANDLE;

    struct CommandQueue {
      VkQueue queue = VK_NULL_HANDLE;
      std::vector<SwapChain> swapchainUpdates;
      std::vector<VkSwapchainKHR> swapchainSubmit;
      std::vector<uint32_t> swapchainSubmitImages;
      std::vector<VkSemaphoreSubmitInfo> waitSemaphoreSubmit;
      std::vector<VkSemaphore> signalSemaphores;
      std::vector<VkSemaphoreSubmitInfo> signalSemaphoreSubmit;
      std::vector<VkCommandBufferSubmitInfo> commandSubmit;

      bool sparse = false;
      std::shared_ptr<std::mutex> locker;

      void submit(VulkanInterface* iface, VkFence fence);
    } queues[QueueType::COUNT];

    struct Allocator {
      VulkanInterface* iface = nullptr;
      std::mutex locker;

      struct Copy {
        VkCommandPool transferPool = VK_NULL_HANDLE;
        VkCommandBuffer transferBuffer = VK_NULL_HANDLE;
        VkCommandPool transitionPool = VK_NULL_HANDLE;
        VkCommandBuffer transitionBuffer = VK_NULL_HANDLE;
        VkFence fence = VK_NULL_HANDLE;
        VkSemaphore semaphores[3] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };
        GPUBuffer uploadBuffer;

        inline bool IsValid() const { return transferBuffer != VK_NULL_HANDLE; }
      };

      std::vector<Copy> freeList;

      void Init(VulkanInterface* iface);
      void Destroy();
      Copy Allocate(size_t staging);
      void Submit(Copy cmd);
    };
    mutable Allocator allocate;

    VkFence frameFence[FRAMEBUFFERS][QueueType::COUNT] = {};

    struct DescriptorBind {
        DescriptorBinds table;
        VulkanInterface* iface;

        std::vector<VkWriteDescriptorSet> descriptorWrite;
        std::vector<VkDescriptorBufferInfo> buffers;
        std::vector<VkDescriptorImageInfo> images;
        std::vector<VkBufferView> views;
        std::vector<VkWriteDescriptorSetAccelerationStructureKHR> accelerationStructures;

        uint32_t uniformOffsets[BIND_CONSTANTS] = {};

        VkDescriptorSet graphics = VK_NULL_HANDLE;
        VkDescriptorSet compute = VK_NULL_HANDLE;

        enum class DirtyFlags : uint32_t {
          NONE = 0,
          DESCRIPTOR = 1,
          OFFSET = 2,

          ALL = DESCRIPTOR | OFFSET
        } dirty = DirtyFlags::NONE;

        void Init(VulkanInterface* iface);
        void Reset();
        void Flush(bool graphics, ThreadCommands cmd);
    };

    struct DescriptorBindPool {
      VulkanInterface* iface;
      VkDescriptorPool pool = VK_NULL_HANDLE;
      uint32_t poolSize = 256;

      void Init(VulkanInterface* iface);
      void Destroy();
      void Reset();
    };

    struct VulkanThreadCommands {
      VkSemaphore semaphore = VK_NULL_HANDLE;
      VkCommandPool pools[FRAMEBUFFERS][QueueType::COUNT] = {};
      VkCommandBuffer buffers[FRAMEBUFFERS][QueueType::COUNT] = {};
      uint32_t bufferIdx = 0;

      QueueType queue = {};
      uint32_t id = 0;
      std::vector<ThreadCommands> cmds;
      std::atomic_bool paused { false };

      DescriptorBind binds;
      DescriptorBindPool bindPools[FRAMEBUFFERS];
      GPULinearAllocator frameAllocators[FRAMEBUFFERS];

      std::vector<std::pair<size_t, VkPipeline>> pipelines;
      const PipelineState* activePSO = nullptr;
      const Shader* activeShader = nullptr;
      const RaytracingPipeline* activeRT = nullptr;

      size_t prevPipelineHash = 0;
      ShadingRate prevShadeRate = {};
      std::vector<SwapChain> prevSwapchains;
      bool PSODirty = false;

      std::vector<VkMemoryBarrier2> memoryBarrier;
      std::vector<VkImageMemoryBarrier2> imageBarrier;
      std::vector<VkBufferMemoryBarrier2> bufferBarrier;
      std::vector<VkAccelerationStructureGeometryKHR> rtBuildGeo;
      std::vector<VkAccelerationStructureBuildRangeInfoKHR> rtBuildRange;

      RenderPassMeta passMeta;
      std::vector<VkImageMemoryBarrier2> renderPassStartBarriers;
      std::vector<VkImageMemoryBarrier2> renderPassEndBarriers;

      void Reset(uint32_t bufIdx) {
          bufferIdx = bufIdx;
          cmds.clear();
          bindPools[bufIdx].Reset();
          binds.Reset();
          frameAllocators[bufIdx].Reset();
          prevPipelineHash = 0;
          activePSO = nullptr;
          activeShader = nullptr;
          activeRT = nullptr;
          PSODirty = false;
          prevShadeRate = ShadingRate::RATE_INVALID;
          prevSwapchains.clear();
          passMeta = {};
          renderPassStartBarriers.clear();
          renderPassEndBarriers.clear();
      }

      constexpr VkCommandPool GetCommandPool() const {
          return pools[bufferIdx][queue];
      }

      constexpr VkCommandBuffer GetCommandBuffer() const {
          return buffers[bufferIdx][queue];
      }
    };

    std::vector<std::unique_ptr<VulkanThreadCommands>> cmds;
    uint32_t cmdCount = 0;
    ShadowEngine::SpinLock cmdLock;

    constexpr VulkanThreadCommands& GetThreadCommands(ThreadCommands cmd) const {
        assert(cmd.isValid());
        return *(VulkanThreadCommands*)cmd.internal;
    }

    struct PSOLayout {
        VkPipelineLayout layout = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> bindless;
        uint32_t firstBindless = 0;
    };

    mutable std::unordered_map<size_t, PSOLayout> PSOcache;
    mutable std::mutex PSOcacheMutex;

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    std::unordered_map<size_t, VkPipeline> pipelines;

    void ValidatePSOs(ThreadCommands cmd);
    void Predraw(ThreadCommands cmd);
    void Predispatch(ThreadCommands cmd);

    static constexpr uint32_t immutableSamplerBegin = 100;
    std::vector<VkSampler> immutableSamplers;

  public:

    VulkanInterface(void* window, Validation validation = Validation::DISABLED, RenderDeviceTypePreference preference = RenderDeviceTypePreference::DISCRETE);
    ~VulkanInterface() override;


    struct MemoryManager {
      VmaAllocator allocator = VK_NULL_HANDLE;
      VmaAllocator externalAllocator = VK_NULL_HANDLE;
      VkDevice device = VK_NULL_HANDLE;
      VkInstance instance;
      size_t frames = 0;
      std::mutex destroyLock;

      struct BindlessHeap {
        VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
        VkDescriptorPool pool = VK_NULL_HANDLE;
        VkDescriptorSet set = VK_NULL_HANDLE;
        std::vector<int> freeList;
        std::mutex lock;

        void Init(VkDevice device, VkDescriptorType type, uint32_t descriptors) {
            descriptors = std::min(descriptors, 500'000u);

            VkDescriptorPoolSize size = {
                .type = type,
                .descriptorCount = descriptors
            };

            VkDescriptorPoolCreateInfo create = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
                .maxSets = 1,
                .poolSizeCount = 1,
                .pPoolSizes = &size,
            };

            VkResult res = vkCreateDescriptorPool(device, &create, nullptr, &pool);
            assert(res == VK_SUCCESS);

            VkDescriptorSetLayoutBinding binding = {
                .binding = 0,
                .descriptorType = type,
                .descriptorCount = descriptors,
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = nullptr
            };


            VkDescriptorBindingFlags bindFlags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

            VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                .bindingCount = 1,
                .pBindingFlags = &bindFlags,
            };

            VkDescriptorSetLayoutCreateInfo layoutCreate = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = &flagsInfo,
                .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
                .bindingCount = 1,
                .pBindings = &binding,
            };

            res = vkCreateDescriptorSetLayout(device, &layoutCreate, nullptr, &setLayout);
            assert(res == VK_SUCCESS);

            VkDescriptorSetAllocateInfo allocateInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &setLayout
            };

            res = vkAllocateDescriptorSets(device, &allocateInfo, &set);
            assert(res == VK_SUCCESS);

            for (int i = 0; i < (int)descriptors; i++)
                freeList.push_back((int)descriptors - i - 1);
        }

        void Destroy(VkDevice device) {
            if (setLayout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
                setLayout = VK_NULL_HANDLE;
            }

            if (pool != VK_NULL_HANDLE) {
                vkDestroyDescriptorPool(device, pool, nullptr);
                pool = VK_NULL_HANDLE;
            }
        }

        int Allocate() {
            lock.lock();
            if (!freeList.empty()) {
                int idx = freeList.back();
                freeList.pop_back();
                lock.unlock();
                return idx;
            } else {
                lock.unlock();
                return -1;
            }
        }

        void Free(int idx) {
            if (idx > 0) {
                lock.lock();
                freeList.push_back(idx);
                lock.unlock();
            }
        }
      };

      BindlessHeap bindlessImages;
      BindlessHeap bindlessUniformTBuffers;
      BindlessHeap bindlessStorageBuffers;
      BindlessHeap bindlessStorageImages;
      BindlessHeap bindlessStorageTBuffers;
      BindlessHeap bindlessSamplers;
      BindlessHeap bindlessRT;

      #define DESTROYER(x, y) \
      std::deque<std::pair<x, size_t>> destroyer_##y
      #define DESTROYER_PAIR(x, y, z) \
      std::deque<std::pair<std::pair<x,y>, size_t>> destroyer_##z

      DESTROYER(VmaAllocation, allocations);
      DESTROYER_PAIR(VkImage, VmaAllocation, images);
      DESTROYER(VkImageView, imageViews);
      DESTROYER_PAIR(VkBuffer, VmaAllocation, buffers);
      DESTROYER(VkBufferView, bufferViews);
      DESTROYER(VkAccelerationStructureKHR, rt);
      DESTROYER(VkSampler, samplers);
      DESTROYER(VkDescriptorPool, descriptorPools);
      DESTROYER(VkDescriptorSetLayout, descriptorSetLayouts);
      DESTROYER(VkDescriptorUpdateTemplate, descriptorSetUpdates);
      DESTROYER(VkShaderModule, shaders);
      DESTROYER(VkPipelineLayout, pipelineLayouts);
      DESTROYER(VkPipeline, pipelines);
      DESTROYER(VkQueryPool, queryPools);
      DESTROYER(VkSwapchainKHR, swapChains);
      DESTROYER(VkSurfaceKHR, surfaces);
      DESTROYER(VkSemaphore, semaphores);
      DESTROYER(VkVideoSessionKHR, videoSessions);
      DESTROYER(VkVideoSessionParametersKHR, videoSessionParameters);
      DESTROYER(int, bindlessImages);
      DESTROYER(int, bindlessUniformTBuffers);
      DESTROYER(int, bindlessStorageBuffers);
      DESTROYER(int, bindlessStorageImages);
      DESTROYER(int, bindlessStorageTBuffers);
      DESTROYER(int, bindlessSamplers);
      DESTROYER(int, bindlessRT);
      #undef DESTROYER
      #undef DESTROYER_PAIR

      ~MemoryManager() {
          bindlessImages.Destroy(device);
          bindlessUniformTBuffers.Destroy(device);
          bindlessStorageBuffers.Destroy(device);
          bindlessStorageImages.Destroy(device);
          bindlessStorageTBuffers.Destroy(device);
          bindlessSamplers.Destroy(device);
          bindlessRT.Destroy(device);

          vmaDestroyAllocator(allocator);
          vmaDestroyAllocator(externalAllocator);
          vkDestroyDevice(device, nullptr);
          vkDestroyInstance(instance, nullptr);
      }

      void Update(size_t frameCount, size_t buffers) {
          const auto processAll = [&](auto&& list, auto&& callback) {
              while (!list.empty())
                  if (list.front().second + buffers < frameCount) {
                      auto item = list.front();
                      list.pop_front();
                      callback(item.first);
                  } else
                      break;
          };

          frames = frameCount;
          destroyLock.lock();

          #define DESTROY(x, y)                         \
          processAll(destroyer_##x, [&](auto& item) {   \
              y;                                        \
          });

          DESTROY(allocations, vmaFreeMemory(allocator, item))

          DESTROY(images, vmaDestroyImage(allocator, item.first, item.second))

          DESTROY(imageViews, vkDestroyImageView(device, item, nullptr))

          DESTROY(buffers, vmaDestroyBuffer(allocator, item.first, item.second))

          DESTROY(bufferViews, vkDestroyBufferView(device, item, nullptr))
          DESTROY(rt, vkDestroyAccelerationStructureKHR(device, item, nullptr))
          DESTROY(samplers, vkDestroySampler(device, item, nullptr))
          DESTROY(descriptorPools, vkDestroyDescriptorPool(device, item, nullptr))
          DESTROY(descriptorSetLayouts, vkDestroyDescriptorSetLayout(device, item, nullptr))
          DESTROY(descriptorSetUpdates, vkDestroyDescriptorUpdateTemplate(device, item, nullptr))
          DESTROY(shaders, vkDestroyShaderModule(device, item, nullptr))
          DESTROY(pipelineLayouts, vkDestroyPipelineLayout(device, item, nullptr))
          DESTROY(pipelines, vkDestroyPipeline(device, item, nullptr))
          DESTROY(queryPools, vkDestroyQueryPool(device, item, nullptr))
          DESTROY(swapChains, vkDestroySwapchainKHR(device, item, nullptr))
          DESTROY(surfaces, vkDestroySurfaceKHR(instance, item, nullptr))
          DESTROY(semaphores, vkDestroySemaphore(device, item, nullptr))
          DESTROY(videoSessions, vkDestroyVideoSessionKHR(device, item, nullptr))
          DESTROY(videoSessionParameters, vkDestroyVideoSessionParametersKHR(device, item, nullptr))

          DESTROY(bindlessImages, bindlessImages.Free(item))
          DESTROY(bindlessUniformTBuffers, bindlessUniformTBuffers.Free(item))
          DESTROY(bindlessStorageBuffers, bindlessStorageBuffers.Free(item))
          DESTROY(bindlessStorageImages, bindlessStorageImages.Free(item))
          DESTROY(bindlessStorageTBuffers, bindlessStorageTBuffers.Free(item))
          DESTROY(bindlessSamplers, bindlessSamplers.Free(item))
          DESTROY(bindlessRT, bindlessRT.Free(item))

          destroyLock.unlock();
      }
    };

    std::shared_ptr<MemoryManager> memoryManager;

    // Supports re-creating a swapchain in-place with a changed meta. The window may be null if sc is not null.
    bool CreateSwapchain(const SwapchainMeta* meta, void* window, SwapChain* sc) const override;
    // Start a buffer on the GPU. Do not read from dest.
    bool CreateBuffer(const GPUBufferMeta* meta, const std::function<void(void* dest)>& callback, GPUBuffer* buffer, const GPUResource* alias = nullptr, size_t aliasOffset = 0) const override;
    // Start a texture, with optional initial data.
    bool CreateTexture(const TextureMeta* meta, const SubresourceMeta* subresource, Texture* tex, const GPUResource* alias, size_t aliasOffset = 0) const override;
    // Start a shader from bytecode. Does not accept source, in glsl or hlsl.
    bool CreateShader(ShaderStage stage, const void* code, size_t size, Shader* shader) const override;
    // Start a sampler for a generic sample type.
    bool CreateSampler(const SamplerMeta* meta, Sampler* sampler) const override;
    // Start a query heap, of query objects to be sent to the GPU.
    bool CreateQueryHeap(const GPUQueryHeapMeta* meta, GPUQueryHeap* heap) const override;
    // Start a PSO. If rpm is passed, PSO will be created immediately, but limited to just that RPM.
    bool CreatePipelineState(const PipelineStateMeta* meta, PipelineState* pso, const RenderPassMeta* rpm = nullptr) const override;
    // Start a ray tracing acceleration structure. Rather useless if the GPU does not support accelerated raytracing.
    bool CreateRayTracingAccelerationStructure(const RaytracingAccelerationMeta* meta, RaytracingAcceleration* accel) const override { return false; };
    // Start a ray tracing pso.
    bool CreateRayTracingPipleineState(const RaytracingPipelineMeta* meta, RaytracingPipeline* pso) const override { return false; }
    // Start a video decoder that runs on hardware.
    bool CreateVideoDecoder(const VideoMeta* meta, VideoDecoder* decoder) const override { return false; }

    // Start a subresource reference for a texture.
    bool CreateSubresource(Texture* tex, SubresourceMeta meta, uint32_t firstSlice, uint32_t sliceCount, uint32_t firstMip, uint32_t mipCount, const ImageFormat* formatChange = nullptr, const ImageAspect* aspect = nullptr, const Swizzle* swizzle = nullptr) override;
    // Start a subresource reference to a buffer.
    bool CreateSubresource(GPUBuffer* buf, SubresourceMeta meta, size_t offset, size_t size = ~0u, const ImageFormat* formatChange = nullptr, uint32_t* strideChange = nullptr) const override;

    // Fetch the descriptor index for a (sub)resource in the current active shader. TODO: is this necessary with bindless?
    int GetDescriptorIdx(const GPUResource* resource, ImageViewType sub, int subIdx = -1) const override;
    // Fetch the descriptor info for a sampler in the current active shader. TODO: is this necessary with bindless?
    int GetDescriptorIdx(const Sampler* sampler) const override;

    // Write a new value into the shading rate data in GPU memory.
    int WriteShadingRateValue(ShadingRate rate, void* data) const override {};
    // Write a new value into the Ray Tracing Acceleration Top Level structure.
    int WriteTLRTAcceleration(const RaytracingAccelerationMeta::TopLevel::Instance* instance, void* dest) const override {}
    // Set a new shader for a specific hit group entry.
    int WriteShaderIdentifier(const RaytracingAccelerationMeta* meta, uint32_t idx, void* data) const override {}

    // For debugging: assign the resource a name ON-GPU
    void SetName(GPUResource* resource, const char* name) const override{}
    void setName(Shader* shader, const char* name) const override {}

    // Start a new command list for the current thread and begin listening.
    ThreadCommands BeginCommands(QueueType queue = QueueType::GRAPHICS) override;

    // Submit all command lists that were created from the current device, across all threads.
    void SubmitCommands() override;

    // Wait for all submitted work to complete.
    void WaitForGPU() const override;

    // Clear pipleline cache - usually when reloading shaders!
    void ClearPipelineCache() override;

    // Fetch how many pipelines are active.
    // Should be pretty low.
    size_t GetPipelineCount() const override;

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
    ShaderFormat GetShaderFormat() const override;

    // Get the Texture resource that represents the current backbuffer (renderable)
    Texture GetBackbuffer(const SwapChain* sc) const override;
    // Get the color space of the current swapchain
    ColorSpace GetColorspace(const SwapChain* sc) const override;
    // Check whether the Swapchain could support HDR.
    bool SwapchainSupportsHDR(const SwapChain* sc) const override;

    // Get the minimum offset alignment for creating subresources in buffers.
    size_t GetMinOffsetAlignment(const GPUBufferMeta* meta) const override {
        size_t alignment = 1;
        if (has_flag(meta->binding, BindFlag::CONSTANT_BUFFER))
            alignment = std::max(alignment, deviceProps2.properties.limits.minUniformBufferOffsetAlignment);
        if (has_flag(meta->flags, ResourceFlags::BUFFER_RAW) || has_flag(meta->flags, ResourceFlags::SPARSE_BUFFER))
            alignment = std::max(alignment, deviceProps2.properties.limits.minStorageBufferOffsetAlignment);
        if (meta->format != ImageFormat::UNKNOWN || has_flag(meta->flags, ResourceFlags::CAST_TYPED))
            alignment = std::max(alignment, deviceProps2.properties.limits.minTexelBufferOffsetAlignment);
        return alignment;
    };

    // Get the total memory available, and the total memory used.
    MemoryUsage GetMemoryUsage() const override {
        MemoryUsage ret;
        VmaBudget budgets[VK_MAX_MEMORY_HEAPS] = {};

        vmaGetHeapBudgets(memoryManager->allocator, budgets);
        for (size_t i = 0; i < memoryProps.memoryProperties.memoryHeapCount; i++) {
            if (memoryProps.memoryProperties.memoryHeaps[i].flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                ret.budget += budgets[i].budget;
                ret.usage += budgets[i].usage;
            }
        }

        return ret;
    };

    // Get the max amount of viewports that can be bound at a time.
    uint32_t GetMaxViewports() const override { return deviceProps2.properties.limits.maxViewports; };

    // Batch update of sparse resources
    void SparesUpdate(QueueType queue, const SparseUpdateCommand* commamds, uint32_t commandCount) override {}

    /**
     * Command list stuff
     */

    // Wait for the other command list to complete.
    void WaitForCommands(ThreadCommands cmd, ThreadCommands waitFor) override;
    // Begin a render pass on the swapchain.
    void BeginRenderPass(const SwapChain* sc, ThreadCommands cmd) override;
    // Begin a render pass on the given image.
    void BeginRenderPass(const RenderPassImage* imgs, uint32_t imageCount, ThreadCommands cmd) override;
    // End a render pass
    void EndRenderPass(ThreadCommands cmd) override;
    // Bind rectangles to the scissor buffer.
    void BindScissorRectangles(const Rect* rects, uint32_t rectCount, ThreadCommands cmd) override;
    // Bind viewports to be rendered
    void BindViewports(const Viewport* viewports, uint32_t vpCount, ThreadCommands cmd) override;
    // Bind a resource to a shader slot
    void BindResource(const GPUResource* res, uint32_t slot, ThreadCommands cmd, int subResource = -1) override;
    // Bind multiple resources to a slot list in order.
    void BindResources(const GPUResource* const* resources, uint32_t startSlot, uint32_t count, ThreadCommands cmd) override;
    // Bind an Unordered Access View to a shader slot
    void BindUAV(const GPUResource* res, uint32_t slot, ThreadCommands cmd, int subResource = -1) override;
    // Bind multiple UAVs to a slot list in order.
    void BindUAVs(const GPUResource* const* resource, uint32_t startSlot, uint32_t count, ThreadCommands cmd) override;
    // Bind a sampler to a shader slot.
    void BindSampler(const Sampler* sampler, uint32_t slot, ThreadCommands cmd) override;
    // Bind a constant buffer to a shader slot.
    void BindConstants(const GPUBuffer* buffer, uint32_t slot, ThreadCommands cmd, size_t offset = 0ull) override;
    // Bind vertex buffers to a shader slot list.
    void BindVertexes(const GPUBuffer* const* buffers, uint32_t slot, uint32_t count, const uint32_t* offsets, ThreadCommands cmd) override;
    // Bind index buffer to a shader slot.
    void BindIndex(const GPUBuffer* buffer, const IndexBufferFormat fmt, size_t offset, ThreadCommands cmd) override;
    // Bind a reference to the stencil buffer.
    void BindStencil(uint32_t value, ThreadCommands cmd) override;
    // Bind a new blend factor.
    void BindBlend(float r, float g, float b, float a, ThreadCommands cmd) override;
    // Bind a new shading rate.
    void BindShadingRate(ShadingRate rate, ThreadCommands cmd) override;
    // Bind a new PSO
    void BindPSO(const PipelineState* pso, ThreadCommands cmd) override;
    // Bind a compute shader for execution
    void BindCompute(const Shader* cs, ThreadCommands cmd) override;
    // Bind new bounds for the depth test.
    void BindDepthBounds(float min, float max, ThreadCommands cmd) override;

    // Draw vertices to the screen from the active vertex buffer. Draws vertexCount vertices, starting from vertex index firstVertex in the buffer.
    void Draw(uint32_t vertexCount, uint32_t firstVertex, ThreadCommands cmd) override;
    // Draw indexed vertexes to the screen, reading from active vertex buffer and active index buffer.
    void DrawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t firstVertex, ThreadCommands cmd) override;
    // Run Draw per instance in the buffer.
    virtual void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance, ThreadCommands cmd) override;
    // Run DrawIndexed per instance in the buffer.
    void DrawIndexedInstanced(uint32_t indexCounts, uint32_t instanceCount, uint32_t firstIndex, int32_t firstVertex, uint32_t firstInstance, ThreadCommands cmd) override;
    // Indirectly draw (let the shader figure out the arguments) an indexed object
    void DrawIndexedIndirect(const GPUBuffer* args, size_t offset, ThreadCommands cmd) override;
    // Indirectly draw an instanced indexed mesh
    void DrawIndexedInstanceIndirect(const GPUBuffer* args, size_t offset, ThreadCommands cmd) override;
    // Indirectly draw a configurable amount of indexed meshes.
    void DrawIndexedIndirectCount(const GPUBuffer* args, size_t offset, const GPUBuffer* count, size_t countOffset, uint32_t maxCount, ThreadCommands cmd) override;
    // Indirectly draw a configurable amount of instanced indirect meshes.
    void DrawIndexedInstancedIndirectCount(const GPUBuffer* args, size_t offset, const GPUBuffer* count, size_t countOffset, uint32_t maxCount, ThreadCommands cmd) override;

    // Dispatch a compute shader in parallel.
    void Dispatch(uint32_t tCountX, uint32_t tCountY, uint32_t tCountZ, ThreadCommands cmd) override;
    // Dispatch a compute shader with its own set of args.
    void DispatchIndirect(const GPUBuffer* args, size_t offset, ThreadCommands cmd) override;
    // Dispatch a compute shader with the vertex and index buffers.
    void DispatchMesh(uint32_t tCountX, uint32_t tCountY, uint32_t tCountZ, ThreadCommands cmd) override {}
    // Dispatch a compute shader with vertex and index buffers passed indirectly.
    void DispatchMeshIndirect(const GPUBuffer* args, size_t offset, ThreadCommands cmd) override {}
    // Dispatch a compute shader with vertex and index buffers passed indirectly with a set number of instances
    void DispatchMeshIndirectCount(const GPUBuffer* args, size_t offset, const GPUBuffer* count, size_t countOffset, uint32_t maxCount, ThreadCommands cmd) override {}

    // Copy a resource from source to dest.
    void CopyResource(const GPUResource* source, const GPUResource* dest, ThreadCommands cmd) override;
    // Copy a buffer from source to dest.
    void CopyBuffer(const GPUBuffer* source, size_t sourceOffset, const GPUBuffer* dest, size_t destOffset, size_t size, ThreadCommands cmd) override;
    // Copy a texture from source to dest.
    void CopyTexture(const Texture* source, const Texture* dest, uint32_t destX, uint32_t destY, uint32_t destZ, uint32_t destMip, uint32_t destSlice, uint32_t sourceMip, uint32_t sourceSlice, ThreadCommands cmd, const Box* sourceBox = nullptr, ImageAspect destAspect = ImageAspect::COLOR, ImageAspect sourceAspect = ImageAspect::COLOR) override;

    // Start a query
    void QueryBegin(const GPUQueryHeap* heap, uint32_t idx, ThreadCommands cmd) override;
    // End a query
    void QueryEnd(const GPUQueryHeap* heap, uint32_t idx, ThreadCommands cmd) override;
    // Fetch the results of the query(s)
    void QueryFetch(const GPUQueryHeap* heap, uint32_t idx, uint32_t count, const GPUBuffer* dest, size_t offset, ThreadCommands cmd) override;
    // Reset an arbitrary number of queries
    void QueryReset(const GPUQueryHeap* heap, uint32_t idx, uint32_t count, ThreadCommands cmd) override {}

    // Barrier a state change for a buffer or texture.
    void Barrier(const BarrierType* barriers, uint32_t num, ThreadCommands cmd) override;

    // Update push constants
    void PushConstants(const void* data, uint32_t size, ThreadCommands cmd, uint32_t offset = 0) override;

    // Start a predication
    void PredicationBegin(const GPUBuffer* buffer, size_t offset, PredicationOp op, ThreadCommands cmd) override {}
    // End a predication
    void PredicationEnd(ThreadCommands cmd) override {}

    // Clear an Unordered Access View resource
    void ClearUAV(const GPUResource* resource, uint32_t value, ThreadCommands cmd) override;

    // Decode a single frame of video
    void VideoDecode(const VideoDecoder* decoder, const VideoDecodeOperation* op, ThreadCommands cmd) override;

    // Build an acceleration structure for RT.
    void BuildRTAcceleration(const RaytracingAcceleration* dest, ThreadCommands cmd) override {}
    // Bind a RT pipeline
    void BindRayTracingPipeline(const RaytracingPipeline* pso, ThreadCommands cmd) override {}
    // Dispatch a compute shader to calculate the rays
    void DispatchRays(const RayDispatch* dispatch, ThreadCommands cmd) override {}

    // Begin an event in the profiler
    void EventBegin(const char* name, ThreadCommands cmd) override;
    // End the most recent even in the profiler
    void EventEnd(ThreadCommands cmd) override;
    // Set a marker in the profiler
    void SetMarker(const char* name, ThreadCommands cmd) override;

    RenderPassMeta GetRenderPassMeta(ThreadCommands cmd) override {
        return GetThreadCommands(cmd).passMeta;
    };

    // Get the allocator to be used for the current frame.
    GPULinearAllocator& GetAllocator(ThreadCommands cmd) override {
        return GetThreadCommands(cmd).frameAllocators[GetBufferIndex()];
    };

  };
}