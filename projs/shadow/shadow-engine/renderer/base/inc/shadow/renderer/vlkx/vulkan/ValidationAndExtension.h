#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include <SDL.h>

class ValidationAndExtension {
public:
	ValidationAndExtension();
	~ValidationAndExtension();

	const std::vector<const char*> requiredValidations = {
		"VK_LAYER_KHRONOS_validation",
        //"VK_LAYER_LUNARG_api_dump"
	};

	VkDebugReportCallbackEXT callback;

	bool checkValidationSupport();

	std::vector<const char*> getRequiredExtensions(SDL_Window* window, bool validationsRequired);
	void setupDebugCallback(bool validationsRequired, VkInstance vulkan);
	void destroy(bool validationsRequired, VkInstance vulkan);


	VkResult createDebugReportCallbackEXT(
		VkInstance vulkan,
		const VkDebugReportCallbackCreateInfoEXT* info,
		const VkAllocationCallbacks* allocator,
		VkDebugReportCallbackEXT* callback) {

		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vulkan, "vkCreateDebugReportCallbackEXT");

		if (func != nullptr) {
			return func(vulkan, info, allocator, callback);
		} else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void destroyDebugReportCallbackEXT(
		VkInstance vulkan,
		const VkDebugReportCallbackEXT callback,
		const VkAllocationCallbacks* allocator) {
		
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vulkan, "vkDestroyDebugReportCallbackEXT");

		if (func != nullptr) {
			func(vulkan, callback, allocator);
		}
	}
};

