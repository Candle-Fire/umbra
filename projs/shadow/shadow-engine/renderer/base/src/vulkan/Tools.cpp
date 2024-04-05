#include <shadow/renderer/vlkx/vulkan/Tools.h>
#include <string>
#include "shadow/renderer/vlkx/vulkan/abstraction/Commands.h"

API VmaAllocator VkTools::allocator;

VkTools::ManagedImage VkTools::createImage(VkFormat format, VkImageUsageFlags flags, VkExtent3D extent) {
    // Set up image metadata
    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_3D;
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
    vmaCreateImage(allocator, &info, &allocateInfo, &image.image, &image.allocation, nullptr);

    return image;
}

VkSampler VkTools::createSampler(VkFilter filters, VkSamplerAddressMode mode, uint32_t mipping, VkDevice dev) {
    VkSamplerCreateInfo info = {
            VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            nullptr, {}, filters, filters,
            VK_SAMPLER_MIPMAP_MODE_LINEAR, mode, mode, mode,
            0, VK_TRUE, 16, VK_FALSE,
            VK_COMPARE_OP_ALWAYS, 0, static_cast<float>(mipping),
            VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE
    };

    VkSampler sampler;
    vkCreateSampler(dev, &info, nullptr, &sampler);

    return sampler;
}

VkImageView VkTools::createImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, uint32_t mipping, uint32_t layers, VkDevice device) {
    // Raw information about the image
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = layers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format = format;

    // Information about the things we want to create - size, mip levels.
    viewInfo.subresourceRange.aspectMask = flags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipping;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = layers;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        throw std::runtime_error("Failed to create texture image view.");

    return imageView;
}

VkTools::ManagedBuffer VkTools::createGPUBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDevice logicalDevice, VkPhysicalDevice physicalDevice, bool hostVisible) {
    // Prepare for creation of a buffer
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    ManagedBuffer buffer {};

    VmaAllocationCreateInfo vmaInfo = {};
    vmaInfo.usage = hostVisible ? VMA_MEMORY_USAGE_CPU_ONLY : VMA_MEMORY_USAGE_GPU_ONLY;
    vmaInfo.requiredFlags = properties;

    // Start the buffer.
    if (VkResult status = vmaCreateBuffer(allocator, &bufferInfo, &vmaInfo, &buffer.buffer, &buffer.allocation, nullptr); status != VK_SUCCESS)
        throw std::runtime_error("Unable to create GPU buffer: " + std::to_string(status));

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

void VkTools::immediateExecute(const std::function<void(const VkCommandBuffer&)>& execute, VulkanDevice* dev) {
    vlkx::ImmediateCommand cmd({ dev->graphicsQueue, dev->queueData.graphics });
    cmd.run(execute);
}

void VkTools::copyGPUBuffer(VkBuffer source, VkBuffer dest, VkDeviceSize length, VulkanDevice* dev) {
    immediateExecute([&](const VkCommandBuffer& commands) {
        // Prepare to copy the data between buffers
        VkBufferCopy copyInfo = {};
        copyInfo.srcOffset = 0;
        copyInfo.dstOffset = 0;
        copyInfo.size = length;

        // Copy the data.
        vkCmdCopyBuffer(commands, source, dest, 1, &copyInfo);
    }, dev);
}