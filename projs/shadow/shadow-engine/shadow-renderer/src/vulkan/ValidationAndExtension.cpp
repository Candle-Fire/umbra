#include <vlkx/vulkan/ValidationAndExtension.h>
#include <SDL.h>
#include <SDL_vulkan.h>

ValidationAndExtension::ValidationAndExtension() {}
ValidationAndExtension::~ValidationAndExtension() {}

bool ValidationAndExtension::checkValidationSupport() {
	// Get number of properties
	uint32_t layers;
	vkEnumerateInstanceLayerProperties(&layers, nullptr);

	// Instantiate a vector of proper size and retrieve data
	std::vector<VkLayerProperties> availableProperties(layers);
	vkEnumerateInstanceLayerProperties(&layers, availableProperties.data());

	// Iterate the validation layers we require
	for (const char* layer : requiredValidations) {
		bool layerFound = false;

		// Search for a match with the ones we have
		for (const auto& property : availableProperties) {
			if (strcmp(layer, property.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		// If any are not found, then we don't support what we need
		if (!layerFound)
			return false;
	}

	// If all are found, then we can continue
	return true;
}

std::vector<const char*> ValidationAndExtension::getRequiredExtensions(SDL_Window* window, bool validationsRequired) {

    unsigned int count;
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);

    std::vector<const char*> extensions = {
            #ifdef __APPLE__
            "VK_KHR_portability_enumeration",
            #endif
            "VK_KHR_get_physical_device_properties2"
    };

    size_t additional_extension_count = extensions.size();
    extensions.resize(additional_extension_count + count);

    SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data() + additional_extension_count);

	if (validationsRequired) {
		extensions.push_back("VK_EXT_debug_report"); // Add debug report if we want to validate
	}

	// Return the new list
	return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objExt,
	uint64_t obj,
    size_t location,
	int32_t code,
	const char* layer,
	const char* message,
	void* user) {

	std::cerr << "Validation from layer " << layer << ": " << message << std::endl;
	return false;
}

void ValidationAndExtension::setupDebugCallback(bool validationRequired, VkInstance vulkan) {
	if (!validationRequired)
		return;

	VkDebugReportCallbackCreateInfoEXT info = {};
	info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	info.pfnCallback = debugCallback;

	if (createDebugReportCallbackEXT(vulkan, &info, nullptr, &callback) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create log dumper.");
	}
}

void ValidationAndExtension::destroy(bool validationRequired, VkInstance vulkan) {
	if (validationRequired)
		destroyDebugReportCallbackEXT(vulkan, callback, nullptr);
}