#pragma once

#include <vlkx/vulkan/ValidationAndExtension.h>
#include <vlkx/vulkan/VulkanDevice.h>
#include <vlkx/vulkan/SwapChain.h>
#include <vlkx/render/framebuffer/RenderPass.h>
#include <vlkx/render/texture/RenderTexture.h>
#include <vlkx/vulkan/CommandBuffer.h>

#include <vulkan/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <vlkx/vulkan/Tools.h>

#include <SDL_vulkan.h>

class VulkanManager {
public:
	VulkanManager();
	~VulkanManager();

#ifdef _DEBUG
	static const bool validationRequired = true;
#else
	static const bool validationRequired = false;
#endif

	// VulkanManager is a singleton class.
	static VulkanManager* instance;
	static VulkanManager* getInstance();

	// Initialize all Vulkan context and prepare validations in debug mode.
	void initVulkan(SDL_Window* window);
	void createAppAndVulkanInstance(bool enableValidation, ValidationAndExtension* validations);

	// Start and end a frame render.
	void startDraw();
	void endDraw();

	// Cleanup after the application has closed.
	void cleanup();

	void useRayTrace() { rayTraceMode = true; }

	VkInstance getVulkan() { return vulkan; }
	VulkanDevice* getDevice() { return device; }
	SwapChain* getSwapchain() { return swapchain; }
	RenderPass* getRenderPass() { return renderPass; }
	VkCommandBuffer getCurrentCommandBuffer() { return currentCommandBuffer; }
	VmaAllocator getAllocator() { return allocator; }
	RenderTexture* getRenderTarget() { return renderTexture; }

private:
    // To keep track of the window during... stuff
    SDL_Window* wnd;

	// To handle the validation of Vulkan API usage (because fuck you, it's your problem now)
	ValidationAndExtension* validators{};
	// To manage interaction with the hardware
	VulkanDevice* device{};
	// To handle the framebuffers
	SwapChain* swapchain{};
	// To handle the timing of rendering
	RenderPass* renderPass{};

	// To handle automatic management of memory.
	VmaAllocator allocator{};

	// The default RenderTexture, mirroring the SwapChain to the viewport.
	RenderTexture* renderTexture;
	// The command buffers used when telling the firmware to do things for us.
	CommandBuffer* buffers{};

	// To manage the Vulkan context that was passed to us by the API
	VkInstance vulkan{};
	// To manage the canvas that was given to us by GLFW
	VkSurfaceKHR surface{};

	// The index of the texture that is currently being used by the GPU.
	uint32_t imageIndex = 0;
	// The command buffer currently being used by the GPU.
	VkCommandBuffer currentCommandBuffer{};

	// The maximum number of frames that can be dealt with at a time.
	const int MAX_FRAMES = 2; // Double-buffering requires two frames in memory
	// Raised when a new image is available
	VkSemaphore newImageSem{};
	// Raised when a render is finished
	VkSemaphore renderDoneSem{};
	// Stores fences for frames that are currently "in flight".
	std::vector<VkFence> inFlight;

	bool rayTraceMode;

};