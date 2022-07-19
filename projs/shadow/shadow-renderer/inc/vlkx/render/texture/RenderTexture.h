#pragma once

#include <vulkan/vulkan.h>
#include <array>
#include <vector>

class RenderTexture {
public:
	// Create all image views and all framebuffers from a set of images, a format, a size, and the current rendering pass.
	virtual void createViewsAndFramebuffer(std::vector<VkImage> images, VkFormat format, VkExtent2D extent, VkRenderPass pass) = 0;
	// Create views for swapChainImages with the given format.
	virtual void createViews(VkFormat format) = 0;
	// Create a framebuffer for the swapChainImages, for the given pass.
	virtual void createFramebuffer(VkExtent2D extent, VkRenderPass pass) = 0;

	virtual VkFramebuffer getFramebuffer(int ID) = 0;

	virtual void destroy() = 0;
};

class SingleRenderTexture : public RenderTexture {
public:
	SingleRenderTexture();
	~SingleRenderTexture();

	// A list of all images involved with this texture (color, normals, etc)
	std::vector<VkImage> swapChainImages;
	// The sizes of all attached images.
	VkExtent2D swapChainImageExtent;

	// Views - mipmaps, portions, crops, etc of the attached images.
	std::vector<VkImageView> swapChainImageViews;
	// Framebuffers containing the images that can be bound and rendered from.
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkFramebuffer getFramebuffer(int ID) override { return swapChainFramebuffers.at(ID); }

	// Create all image views and all framebuffers from a set of images, a format, a size, and the current rendering pass.
	void createViewsAndFramebuffer(std::vector<VkImage> images, VkFormat format, VkExtent2D extent, VkRenderPass pass) override;
	// Create views for swapChainImages with the given format.
	void createViews(VkFormat format) override;
	// Create a framebuffer for the swapChainImages, for the given pass.
	void createFramebuffer(VkExtent2D extent, VkRenderPass pass) override;

	void destroy() override;
};