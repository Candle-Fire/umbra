#pragma once

#include <renderer/Interface.h>
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include "shadow/util/Synchronization.h"
#include <atomic>

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
    //std::vector<VkQueueFamilyVideoPropertiesKHR> queueFamiliesVideo;
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

    };

  };
}