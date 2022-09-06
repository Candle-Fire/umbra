#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <set>

#include <vlkx/vulkan/ValidationAndExtension.h>

struct SwapChainMeta {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> modes;
};

struct QueueFamilies {
	int graphics = -1;
	int presentation = -1;

	bool present() {
		return graphics >= 0 && presentation >= 0;
	}
};

class VulkanDevice {
public:
	VulkanDevice();
	~VulkanDevice();

	/** Physical Devices **/
	VkPhysicalDevice physical;
    VkPhysicalDeviceLimits limits;
	SwapChainMeta swapChain;
	QueueFamilies queueData;

	std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	void choosePhysicalDevice(VkInstance* vulkan, VkSurfaceKHR surface);
	bool isSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
	bool isSupported(VkPhysicalDevice device);

	SwapChainMeta checkSwapchain(VkPhysicalDevice device, VkSurfaceKHR surface);
	QueueFamilies checkQueues(VkPhysicalDevice device, VkSurfaceKHR surface);

	QueueFamilies getQueues() { return queueData; }

	/** Logical Devices **/
	VkDevice logical;
	VkQueue graphicsQueue;
	VkQueue presentationQueue;

	void createLogicalDevice(VkSurfaceKHR surface, bool validationRequired, ValidationAndExtension* validators);
	void destroy();
};