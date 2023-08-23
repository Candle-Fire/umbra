#pragma once

#include <vlkx/vulkan/ValidationAndExtension.h>
#include <vlkx/vulkan/VulkanDevice.h>

#include <vulkan/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <SDL_vulkan.h>
#include <core/Module.h>
#include "SwapChain.h"
#include "event-bus/events.h"
#include "event-bus/render_events.h"

namespace vlkx { class ScreenRenderPassManager; }

class VulkanModule : ShadowEngine::Module {
  SHObject_Base(VulkanModule);
  public:

    VulkanModule() { instance = this; }

    ~VulkanModule() override;

#ifdef _DEBUG
    static const bool validationRequired = true;
#else
    static const bool validationRequired = false;
#endif

    void Init() override;

    void PreInit() override;

    void Recreate();

    void Destroy() override;

    // VulkanModule is a singleton class.
    static VulkanModule *instance;

    static VulkanModule *getInstance();

    // Initialize all Vulkan context and prepare validations in debug mode.
    void initVulkan(SDL_Window *window);

    void createAppAndVulkanInstance(bool enableValidation, ValidationAndExtension *validations);

    // Cleanup after the application has closed.
    void cleanup();

    VkInstance getVulkan() { return vulkan; }

    VulkanDevice *getDevice() { return device; }

    SwapChain *getSwapchain() { return swapchain; }

    VmaAllocator getAllocator() { return allocator; }

  private:
    // To handle the validation of Vulkan API usage
    ValidationAndExtension *validators{};
    // To manage interaction with the hardware
    VulkanDevice *device{};
    // To handle the framebuffers
    SwapChain *swapchain{};
    // To handle automatic management of memory.
    VmaAllocator allocator{};
    // To manage the Vulkan context that was passed to us by the API
    VkInstance vulkan{};
    // To manage the canvas that was given to us by GLFW
    VkSurfaceKHR surface{};
};