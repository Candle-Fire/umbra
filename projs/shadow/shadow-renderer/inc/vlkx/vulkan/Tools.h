#pragma once

#include <stdexcept>
#include <functional>

#include <vulkan/vulkan.h>
#include <vlkx/vulkan/VulkanManager.h>

#include <vulkan/vk_mem_alloc.h>

namespace VkTools {
    extern VmaAllocator             g_allocator;
    extern VkInstance               g_Instance;
    extern VkPhysicalDevice         g_PhysicalDevice;
    extern VkDevice                 g_Device;
    extern uint32_t                 g_QueueFamily;
    extern VkQueue                  g_Queue;
    extern VkDebugReportCallbackEXT g_DebugReport;

    struct ManagedImage {
        VkImage image;
        VmaAllocation allocation;
    };

    struct ManagedBuffer {
        VkBuffer buffer;
        VmaAllocation allocation;
    };

    ManagedImage createImage(VkFormat format, VkImageUsageFlags flags, VkExtent3D extent, VkDevice device);
    VkSampler createSampler(VkFilter filters, VkSamplerAddressMode mode);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, VkDevice device);
    ManagedBuffer createGPUBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDevice logicalDevice, VkPhysicalDevice physicalDevice);
    uint32_t findMemoryIndex(uint32_t type, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);
    VkCommandBuffer createTempCommandBuffer(VkCommandPool pool, VkDevice logical);
    void executeAndDeleteTempBuffer(VkCommandBuffer buffer, VkCommandPool pool, VkQueue queue, VkDevice logicalDevice);
    void copyGPUBuffer(VkBuffer source, VkBuffer dest, VkDeviceSize length, VkDevice logical, VkQueue graphicsQueue, uint32_t queueIndex);


    #ifdef VKTOOLS_IMPLEMENTATION
    ManagedImage createImage(VkFormat format, VkImageUsageFlags flags, VkExtent3D extent, VkDevice device) {
        // Set up image metadata
        VkImageCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext = nullptr;
        info.format = format;
        info.extent = extent;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = flags;

        // Prepare the managed image
        ManagedImage image {};

        // Set up image allocation
        VmaAllocationCreateInfo allocateInfo = {};
        allocateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        // Allocate + create the image
        vmaCreateImage(g_allocator, &info, &allocateInfo, &image.image, &image.allocation, nullptr);

        return image;

    }

    VkSampler createSampler(VkFilter filters, VkSamplerAddressMode mode) {
        VkSamplerCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.pNext = nullptr;
        info.magFilter = filters;
        info.minFilter = filters;
        info.addressModeU = mode;
        info.addressModeV = mode;
        info.addressModeW = mode;

        VkSampler sampler;
        vkCreateSampler(g_Device, &info, nullptr, &sampler);

        return sampler;
    }

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, VkDevice device) {
        // Raw information about the image
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;

        // Information about the things we want to create - size, mip levels.
        viewInfo.subresourceRange.aspectMask = flags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
            throw std::runtime_error("Failed to create texture image view.");

        return imageView;
    }

    ManagedBuffer createGPUBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDevice logicalDevice, VkPhysicalDevice physicalDevice) {
        // Prepare for creation of a buffer
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        ManagedBuffer buffer;

        VmaAllocationCreateInfo vmaInfo = {};
        vmaInfo.usage = VMA_MEMORY_USAGE_AUTO;
        vmaInfo.requiredFlags = properties;
        vmaInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        // Create the buffer.
        if (vmaCreateBuffer(g_allocator, &bufferInfo, &vmaInfo, &buffer.buffer, &buffer.allocation, nullptr) != VK_SUCCESS)
            throw std::runtime_error("Unable to create GPU buffer");

        return buffer;
    }

    uint32_t findMemoryIndex(uint32_t type, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice) {
        // Get the physical properties of the device.
        VkPhysicalDeviceMemoryProperties physProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physProperties);

        // Iterate the device and search for a suitable index
        for (uint32_t i = 0; i < physProperties.memoryTypeCount; i++)
            // If the type matches, and the properties are what we desire, then ship it.
            if ((type & (1 << i)) && ((physProperties.memoryTypes[i].propertyFlags & properties) == properties))
                return i;

        throw std::runtime_error("Unable to find a suitable memory type on the physical device.");
    }

    VkCommandBuffer createTempCommandBuffer(VkCommandPool pool, VkDevice logical) {
        // Prepare to allocate a command buffer
        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandPool = pool;
        allocateInfo.commandBufferCount = 1;

        // Allocate the buffer
        VkCommandBuffer buffer;
        vkAllocateCommandBuffers(logical, &allocateInfo, &buffer);

        // Prepare to begin the new buffer.
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        // Begin listening on the new buffer.
        vkBeginCommandBuffer(buffer, &beginInfo);

        return buffer;
    }

    void executeAndDeleteTempBuffer(VkCommandBuffer buffer, VkCommandPool pool, VkQueue queue, VkDevice logicalDevice) {
        // Stop listening on the buffer
        vkEndCommandBuffer(buffer);

        // Prepare to execute the commands in the buffer
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &buffer;

        // Submit the commands to be executed
        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

        // Wait for the GPU to finish executing
        vkQueueWaitIdle(queue);

        // Delete the now unusable buffers
        vkFreeCommandBuffers(logicalDevice, pool, 1, &buffer);
    }

    void copyGPUBuffer(VkBuffer source, VkBuffer dest, VkDeviceSize length, VkDevice logical, VkQueue graphicsQueue, uint32_t queueIndex) {

        // Prepare to create a temporary command pool.
        VkCommandPool pool;
        VkCommandPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.queueFamilyIndex = queueIndex;
        poolCreateInfo.flags = 0;

        // Create the pool
        if (vkCreateCommandPool(logical, &poolCreateInfo, nullptr, &pool) != VK_SUCCESS)
            throw std::runtime_error("Unable to allocate a temporary command pool");

        // Allocate a buffer
        VkCommandBuffer commands = createTempCommandBuffer(pool, logical);

        // ------ Commands are saved into the commands field ------ //

        // Prepare to copy the data between buffers
        VkBufferCopy copyInfo = {};
        copyInfo.srcOffset = 0;
        copyInfo.dstOffset = 0;
        copyInfo.size = length;

        // Copy the data.
        vkCmdCopyBuffer(commands, source, dest, 1, &copyInfo);

        // ------ Commands are no longer saved into the commands field ------ //

        executeAndDeleteTempBuffer(commands, pool, graphicsQueue, logical);

        // Cleanup the temporary buffer and pool we created
        vkDestroyCommandPool(logical, pool, nullptr);
    }

    #endif
}