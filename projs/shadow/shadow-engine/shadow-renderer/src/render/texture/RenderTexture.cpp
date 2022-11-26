#include <vlkx/render/texture/RenderTexture.h>
#include <vlkx/vulkan/VulkanManager.h>
#include <vlkx/vulkan/Tools.h>

SingleRenderTexture::SingleRenderTexture() {}
SingleRenderTexture::~SingleRenderTexture() {}

void SingleRenderTexture::createViewsAndFramebuffer(std::vector<VkImage> images, VkFormat format, VkExtent2D extent, VkRenderPass pass) {
	// Initialize members
	this->swapChainImages = images;
	this->swapChainImageExtent = extent;

	createViews(format);
	createFramebuffer(extent, pass);
}

void SingleRenderTexture::createViews(VkFormat format) {
	// Prepare maximum size
	swapChainImageViews.resize(swapChainImages.size());

	// Iterate images and create views for each.
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		swapChainImageViews[i] = VkTools::createImageView(swapChainImages[i], format, VK_IMAGE_ASPECT_COLOR_BIT, VulkanManager::getInstance()->getDevice()->logical);
	}
}

void SingleRenderTexture::createFramebuffer(VkExtent2D extent, VkRenderPass pass) {
	// Prepare maximum size
	swapChainFramebuffers.resize(swapChainImageViews.size());

	// Iterate views and create a framebuffer for each.
	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		// Create an array with the image view as an attachment.
		std::array<VkImageView, 1> attachments = {
			swapChainImageViews[i]
		};

		// Prepare the creation of a new framebuffer.
		// One attachment, width and height from the extent, and one layer (one viewport)
		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass = pass;
		info.attachmentCount = static_cast<uint32_t>(attachments.size());
		info.pAttachments = attachments.data();
		info.width = extent.width;
		info.height = extent.height;
		info.layers = 1;
		
		// Create the framebuffer
		if (vkCreateFramebuffer(VulkanManager::getInstance()->getDevice()->logical, &info, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("Unable to create framebuffer for a texture.");
	}
}

void SingleRenderTexture::destroy() {
	// Destroy Image Views first
	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(VulkanManager::getInstance()->getDevice()->logical, imageView, nullptr);
	}

	// Framebuffers
	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(VulkanManager::getInstance()->getDevice()->logical, framebuffer, nullptr);
	}

}