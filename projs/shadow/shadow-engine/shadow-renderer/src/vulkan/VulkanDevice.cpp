#include <vlkx/vulkan/VulkanDevice.h>
#include "spdlog/spdlog.h"

VulkanDevice::VulkanDevice() : physical(VK_NULL_HANDLE), logical(VK_NULL_HANDLE), graphicsQueue(VK_NULL_HANDLE), presentationQueue(VK_NULL_HANDLE) {}
VulkanDevice::~VulkanDevice() = default;

void VulkanDevice::choosePhysicalDevice(VkInstance* vulkan, VkSurfaceKHR surface) {
	// Count devices
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(*vulkan, &deviceCount, nullptr);

	// Sanity check
	if (deviceCount == 0) {
		throw std::runtime_error("Vulkan not supported on this system. No Devices available");
	}

    spdlog::debug("Found " + std::to_string(deviceCount) + " devices that are Vulkan-compatible.");

	// Gather devices
	std::vector<VkPhysicalDevice> physicals(deviceCount);
	vkEnumeratePhysicalDevices(*vulkan, &deviceCount, physicals.data());

	// Enumerate devices
	std::string finalDeviceName;
    spdlog::debug("Device List");
	for (const auto& device : physicals) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(device, &props);
        limits = props.limits;

        bool dedicated = props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        spdlog::debug(std::string("Device: ") + props.deviceName + ", discrete: " + (dedicated ? "yes" : "no"));
		if (physical == VK_NULL_HANDLE && isSuitable(device, surface) || dedicated && isSuitable(device, surface)) {
			finalDeviceName = props.deviceName;
			physical = device;
		}
	}

    spdlog::debug("Using device " + std::string(finalDeviceName) + ".");
	// Sanity check that at least one was found.
	if (physical == VK_NULL_HANDLE)
		throw std::runtime_error("No suitable GPU found");
}

bool VulkanDevice::isSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
	// Find queues

	QueueFamilies families = checkQueues(device, surface);

	bool supported = isSupported(device);
	bool swapChainWorks = false;

	if (supported) {
		swapChain = checkSwapchain(device, surface);
		swapChainWorks = !swapChain.formats.empty() && !swapChain.modes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return families.present() && supported && swapChainWorks && supportedFeatures.samplerAnisotropy;
}

QueueFamilies VulkanDevice::checkQueues(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilies families;
	// Enumerate queues
	uint32_t queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);

	std::vector<VkQueueFamilyProperties> queues(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, queues.data());

	// Find a valid graphics (drawing) and presentation (display) queue
	int i = 0;
	for (const auto& family : queues) {
		// If the graphics bit is set, this is a valid graphics queue
		if (family.queueCount > 0 && family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			families.graphics = i;

		// Ask Vulkan if this family suppots displaying to the surface from this device
		VkBool32 presentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);
		if (family.queueCount > 0 && presentationSupport)
			families.presentation = i;

		// If we have a valid graphics and presentation queue, we can stop searching
		if (families.present())
			break;

		i++;
	}

	return families;
}

bool VulkanDevice::isSupported(VkPhysicalDevice device) {
	// Enumerate extensions
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

	// Filter for the ones we have
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
	for (const auto& extension : extensions) {
		// Remove the ones we have from the ones we need
		requiredExtensions.erase(extension.extensionName);
	}

	// If we have every needed extension, then we're good to go
	return requiredExtensions.empty();
	
}

SwapChainMeta VulkanDevice::checkSwapchain(VkPhysicalDevice device, VkSurfaceKHR surface) {
	SwapChainMeta meta;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &meta.capabilities);

	// Check display formats (ARGB, RBGA, etc)
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		meta.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, meta.formats.data());
	}
	// Check Presentation modes (bit depth, etc)
	uint32_t modeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modeCount, nullptr);

	if (modeCount != 0) {
		meta.modes.resize(modeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modeCount, meta.modes.data());
	}

	return meta;
}

void VulkanDevice::createLogicalDevice(VkSurfaceKHR surface, bool validationRequired, ValidationAndExtension* validator) {

	// Get the queue data, prepare it for the logical device
	QueueFamilies families = checkQueues(physical, surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreation;
	std::set<int> queues = { families.graphics, families.presentation };

	float priority = 1;
	for (int family : queues) {
		VkDeviceQueueCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		createInfo.queueFamilyIndex = family;
		createInfo.queueCount = 1;
		createInfo.pQueuePriorities = &priority;
		queueCreation.push_back(createInfo);
	}

	// Prepare the characteristics of the device we want
	VkPhysicalDeviceFeatures features = {};
	features.samplerAnisotropy = VK_TRUE;

	// Prepare the device for construction
	VkDeviceCreateInfo device = {};
	device.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device.pQueueCreateInfos = queueCreation.data();
	device.queueCreateInfoCount = static_cast<uint32_t>(queueCreation.size());
	device.pEnabledFeatures = &features;
	device.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	device.ppEnabledExtensionNames = deviceExtensions.data();

	if (validationRequired) {
		device.enabledLayerCount = static_cast<uint32_t>(validator->requiredValidations.size());
		device.ppEnabledLayerNames = validator->requiredValidations.data();
	} else {
		device.enabledLayerCount = 0;
	}

	// Create the device

	if (vkCreateDevice(physical, &device, nullptr, &logical) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logical device");
	}

	// Fetch the proper queue metadata from the GPU
	queueData = families;
	vkGetDeviceQueue(logical, families.graphics, 0, &graphicsQueue);
	vkGetDeviceQueue(logical, families.presentation, 0, &presentationQueue);
}

void VulkanDevice::destroy() {
	vkDestroyDevice(logical, nullptr);
}