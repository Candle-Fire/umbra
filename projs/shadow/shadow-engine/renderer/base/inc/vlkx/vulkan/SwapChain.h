#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include <algorithm>
#include "vlkx/vulkan/abstraction/Image.h"

class SwapChain {
public:
	SwapChain();
	~SwapChain();

	VkSwapchainKHR swapChain;
	VkFormat format;
	VkExtent2D extent;

	std::vector<std::unique_ptr<vlkx::Image>> images;
    std::unique_ptr<vlkx::Image> multisampleImg;

	VkSurfaceFormatKHR chooseFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseMode(const std::vector<VkPresentModeKHR>& availableModes);
	VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	void create(VkSurfaceKHR surface);
	void destroy();
};