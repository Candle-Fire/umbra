#pragma once

#include <stdexcept>
#include <functional>

#include <vulkan/vulkan.h>
#include <vulkan/vk_mem_alloc.h>
#include "VulkanDevice.h"

#ifdef EXPORTING_SH_ENGINE
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif

namespace VkTools {
    struct ManagedImage {
        VkImage image;
        VmaAllocation allocation;
    };

    struct ManagedBuffer {
        VkBuffer buffer;
        VmaAllocation allocation;
    };

    extern API VmaAllocator allocator;

    ManagedImage createImage(VkFormat format, VkImageUsageFlags flags, VkExtent3D extent);

    VkSampler createSampler(VkFilter filters, VkSamplerAddressMode mode, uint32_t mipping, VkDevice device);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, uint32_t mipping, uint32_t layers,
                                VkDevice device);

    ManagedBuffer createGPUBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                  VkDevice logicalDevice, VkPhysicalDevice physicalDevice, bool hostVisible = true);

    void immediateExecute(const std::function<void(const VkCommandBuffer &)> &execute, VulkanDevice *dev);

    void copyGPUBuffer(VkBuffer source, VkBuffer dest, VkDeviceSize length, VulkanDevice *dev);

}