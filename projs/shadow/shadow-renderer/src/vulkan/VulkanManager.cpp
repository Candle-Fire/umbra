#define VMA_IMPLEMENTATION

#include <vulkan/vk_mem_alloc.h>

#define VKTOOLS_IMPLEMENTATION

#include <vlkx/vulkan/Tools.h>

#include <vlkx\vulkan\VulkanManager.h>

VulkanManager::VulkanManager() { rayTraceMode = false; }

VulkanManager::~VulkanManager() = default;

VulkanManager* VulkanManager::instance = nullptr;


VmaAllocator             VkTools::g_allocator;
VkInstance               VkTools::g_Instance = VK_NULL_HANDLE;
VkPhysicalDevice         VkTools::g_PhysicalDevice = VK_NULL_HANDLE;
VkDevice                 VkTools::g_Device = VK_NULL_HANDLE;
uint32_t                 VkTools::g_QueueFamily = (uint32_t)-1;
VkQueue                  VkTools::g_Queue = VK_NULL_HANDLE;
VkDebugReportCallbackEXT VkTools::g_DebugReport = VK_NULL_HANDLE;

VulkanManager* VulkanManager::getInstance() {
    return VulkanManager::instance != nullptr ? VulkanManager::instance
                                              : (VulkanManager::instance = new VulkanManager());
}

void VulkanManager::createAppAndVulkanInstance(bool enableValidation, ValidationAndExtension* validations) {
    VkApplicationInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.pApplicationName = "Sup";
    info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    info.pEngineName = "Infinity Drive";
    info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &info;

    auto extensions = validations->getRequiredExtensions(wnd, true);
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = &extensions[0];

    auto layers = validations->requiredValidations;
    if (enableValidation) {
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        instanceInfo.ppEnabledLayerNames = &layers[0];
    } else {
        instanceInfo.enabledLayerCount = 0;
    }

    auto status = vkCreateInstance(&instanceInfo, nullptr, &vulkan);
    if (status != VK_SUCCESS) {
        throw std::runtime_error("Failed to initialize Vulkan: " + std::to_string(status));
    }

}

void VulkanManager::initVulkan(SDL_Window* window) {
    wnd = window;
    validators = new ValidationAndExtension();

    if (!validators->checkValidationSupport())
        throw std::runtime_error("Validation not available");

    createAppAndVulkanInstance(validationRequired, validators);

    validators->setupDebugCallback(validationRequired, vulkan);

    if (SDL_Vulkan_CreateSurface(window, vulkan, &surface) != SDL_TRUE)
        throw std::runtime_error("Unable to create Vulkan Surface");

    this->device = new VulkanDevice();
    this->device->choosePhysicalDevice(&vulkan, surface);
    this->device->createLogicalDevice(surface, validationRequired, validators);

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = this->device->physical;
    allocatorInfo.device = this->device->logical;
    allocatorInfo.instance = this->vulkan;
    vmaCreateAllocator(&allocatorInfo, &this->allocator);
    VkTools::g_allocator = this->allocator;
    VkTools::g_PhysicalDevice = this->device->physical;
    VkTools::g_Device = this->device->logical;
    VkTools::g_Instance = this->vulkan;

    this->swapchain = new SwapChain();
    this->swapchain->create(surface);

    this->renderPass = new RenderPass();

    // Set up for vertex rendering
    this->renderPass->createVertexRenderPass(swapchain->format);
    this->renderTexture = new SingleRenderTexture();

    this->renderTexture->createViewsAndFramebuffer(swapchain->images, swapchain->format,
        swapchain->extent,renderPass->pass
    );

    this->buffers = new CommandBuffer();
    this->buffers->createCommandPoolAndBuffers(swapchain->images.size());

    // Create semaphores for render events
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(device->logical, &semaphoreInfo, nullptr, &newImageSem);
    vkCreateSemaphore(device->logical, &semaphoreInfo, nullptr, &renderDoneSem);

    // Create fences for the frames
    inFlight.resize(MAX_FRAMES);
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES; i++) {
        if (vkCreateFence(device->logical, &fenceInfo, nullptr, &inFlight[i]) != VK_SUCCESS)
            throw std::runtime_error("Unable to create fence for a frame");
    }

    std::cout << "Vulkan Initialization Finished" << std::endl;
}

void VulkanManager::startDraw() {
    // Prepare for a new frame to start
    vkAcquireNextImageKHR(device->logical, swapchain->swapChain,
        std::numeric_limits<uint64_t>::max(), newImageSem,VK_NULL_HANDLE, &imageIndex
    );

    vkWaitForFences(device->logical, 1, &inFlight[imageIndex], VK_TRUE,
        std::numeric_limits<uint64_t>::max()
    );

    vkResetFences(device->logical, 1, &inFlight[imageIndex]);

    // Fetch the next command buffer
    currentCommandBuffer = buffers->buffers[imageIndex];
    buffers->beginCommandBuffer(currentCommandBuffer);

    // Setup render pass; setup clear color
    VkClearValue clearColor = { 1.0f, 0.0f, 0.0f, 1.0f }; // Red

    // Execute render pass
    renderPass->beginRenderPass({ clearColor }, currentCommandBuffer, dynamic_cast<SingleRenderTexture*>(renderTexture)->swapChainFramebuffers[imageIndex], dynamic_cast<SingleRenderTexture*>(renderTexture)->swapChainImageExtent);
}

void VulkanManager::endDraw() {
    // End command buffer first
    renderPass->endRenderPass(currentCommandBuffer);
    buffers->endCommandBuffer(currentCommandBuffer);

    // Prepare to submit all draw commands to the GPU
    VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &currentCommandBuffer;
    submitInfo.pWaitDstStageMask = waitStages;
    // Wait for the New Image semaphore, and signal the Render Done semaphore when finished
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &newImageSem;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderDoneSem;

    // Submit.
    vkQueueSubmit(VulkanManager::getInstance()->getDevice()->graphicsQueue, 1, &submitInfo, inFlight[imageIndex]);

    // Prepare to show the drawn frame on the screen.
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain->swapChain;
    presentInfo.pImageIndices = &imageIndex;
    // Wait until render is finished before presenting.
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderDoneSem;

    // Show.
    vkQueuePresentKHR(VulkanManager::getInstance()->getDevice()->presentationQueue, &presentInfo);

    // Wait for the GPU to catch up
    vkQueueWaitIdle(VulkanManager::getInstance()->getDevice()->presentationQueue);
}

void VulkanManager::cleanup() {
    // Wait for the GPU to not be busy
    vkDeviceWaitIdle(VulkanManager::getInstance()->getDevice()->logical);

    // Destroy our own data
    vkDestroySemaphore(device->logical, renderDoneSem, nullptr);
    vkDestroySemaphore(device->logical, newImageSem, nullptr);

    for (size_t i = 0; i < MAX_FRAMES; i++) {
        vkDestroyFence(device->logical, inFlight[i], nullptr);
    }

    buffers->destroy();
    renderTexture->destroy();
    renderPass->destroy();
    swapchain->destroy();

    // Destroy the Vulkan Device
    VulkanManager::getInstance()->getDevice()->destroy();

    // Delete the layer validators.
    validators->destroy(validationRequired, vulkan);

    // Delete the surface and Vulkan instance.
    vkDestroySurfaceKHR(vulkan, surface, nullptr);
    vkDestroyInstance(vulkan, nullptr);

    vmaDestroyAllocator(allocator);

    // Delete allocated memory for our own data.
    delete buffers;
    delete renderTexture;
    delete renderPass;
    delete swapchain;
    delete device;
    delete validators;
}