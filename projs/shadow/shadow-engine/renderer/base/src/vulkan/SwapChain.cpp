#include <shadow/renderer/vlkx/vulkan/SwapChain.h>
#include <shadow/renderer/vlkx/vulkan/VulkanModule.h>
#include "spdlog/spdlog.h"

SwapChain::SwapChain() {}
SwapChain::~SwapChain() {}

VkSurfaceFormatKHR SwapChain::chooseFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	// Check if we don't have any options
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
		// Default to BGRA, sRGB
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	for (const auto& format : availableFormats) {
		// Prefer BGRA sRGB if it's available
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return format;
	}
	
	// If BGRA sRGB isn't an option, choose whatever Vulkan thinks is the best option.
	return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseMode(const std::vector<VkPresentModeKHR>& availableModes) {
	
	// We want Mailbox, Immediate or FIFO, in that order.
	VkPresentModeKHR mode = availableModes[0];
	if (mode == VK_PRESENT_MODE_MAILBOX_KHR || mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		return mode;

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	// If our extent is valid, use it
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;
	else {
		// Start a new 1280x720 extent and use that
		VkExtent2D size = { 1280, 720 };
		size.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, size.width));
		size.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, size.height));
		return size;
	}
}

void SwapChain::create(VkSurfaceKHR surface) {
	SwapChainMeta info = VulkanModule::getInstance()->getDevice()->swapChain;

	VkSurfaceFormatKHR chosenFormat = chooseFormat(info.formats);
	VkPresentModeKHR chosenMode = chooseMode(info.modes);
	VkExtent2D chosenExtent = chooseExtent(info.capabilities);

	// use the max if it's set, otherwise the minimum
	uint32_t imageCount = std::max(info.capabilities.minImageCount, (uint32_t) 2);
    spdlog::debug("SwapChain has " + std::to_string(imageCount) + " images.");
	
	// Prepare the creation data
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = chosenFormat.format;
	createInfo.imageColorSpace = chosenFormat.colorSpace;
	createInfo.imageExtent = chosenExtent;
	createInfo.imageArrayLayers = 1; // 2 for VR
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilies queues = VulkanModule::getInstance()->getDevice()->getQueues();
	uint32_t indices[] = { static_cast<uint32_t>(queues.graphics), static_cast<uint32_t>(queues.presentation) };

	if (queues.graphics != queues.presentation) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = indices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = info.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = chosenMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// Start the swap-chain
	if (vkCreateSwapchainKHR(VulkanModule::getInstance()->getDevice()->logical, &createInfo, nullptr, &swapChain))
		throw std::runtime_error("Failed to create swap-chain");

    // Set members
    format = chosenFormat.format;
    extent = chosenExtent;

	// Fetch our images from the swapchain

    uint32_t swapchainImgCount = 0;
	vkGetSwapchainImagesKHR(VulkanModule::getInstance()->getDevice()->logical, swapChain, &swapchainImgCount, nullptr);

    std::vector<VkImage> swapchainImgs(swapchainImgCount);
	vkGetSwapchainImagesKHR(VulkanModule::getInstance()->getDevice()->logical, swapChain, &swapchainImgCount, swapchainImgs.data());

    images.resize(0);
    images.reserve(imageCount);
    for (const auto& img : swapchainImgs) {
        images.emplace_back(std::make_unique<vlkx::SwapchainImage>(img, extent, format));
    }

}

void SwapChain::destroy() {
	vkDestroySwapchainKHR(VulkanModule::getInstance()->getDevice()->logical, swapChain, nullptr);
    /*for (auto & image : images)
        image.reset();
    multisampleImg.reset(); */
}