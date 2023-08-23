#define VMA_IMPLEMENTATION

#include <vulkan/vk_mem_alloc.h>
#include <vlkx/vulkan/Tools.h>

#include <vlkx\vulkan\VulkanModule.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "core/ShadowApplication.h"
#include "core/SDL2Module.h"
#include "vlkx/render/render_pass/ScreenRenderPass.h"
#include <vlkx/vulkan/SwapChain.h>
#include "core/module-manager-v2.h"
#include "event-bus/render_events.h"

#include <functional>

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

SHObject_Base_Impl(VulkanModule)

MODULE_ENTRY(VulkanModule, VulkanModule)

std::unique_ptr<vlkx::ScreenRenderPassManager> renderPass;

std::unique_ptr<vlkx::RenderCommand> editorRenderCommands;

VulkanModule::~VulkanModule() = default;

VulkanModule *VulkanModule::instance = nullptr;

VulkanModule *VulkanModule::getInstance() {
    return VulkanModule::instance != nullptr ? VulkanModule::instance
                                             : (VulkanModule::instance = new VulkanModule());
}

void VulkanModule::Recreate() {
    vkDeviceWaitIdle(device->logical);

    device->swapChain = device->checkSwapchain(device->physical, surface);

    if (device->swapChain.capabilities.currentExtent.width == 0
        && device->swapChain.capabilities.currentExtent.height == 0) {
        []() {
            SDL_Event event;
            while (true) {
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_WINDOWEVENT
                        && (event.window.event == SDL_WINDOWEVENT_MAXIMIZED
                            || event.window.event == SDL_WINDOWEVENT_SHOWN
                            || event.window.event == SDL_WINDOWEVENT_RESIZED
                            || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED
                            || event.window.event == SDL_WINDOWEVENT_RESTORED))
                        return;
                }
            }
        }();
    }

    device->swapChain = device->checkSwapchain(device->physical, surface);

    swapchain->destroy();
    swapchain->create(surface);

    renderPass->initializeRenderPass();
}

void VulkanModule::PreInit() {
    spdlog::info("Vulkan Renderer Module loading..");

    auto shApp = ShadowEngine::ShadowApplication::Get();

    ShadowEngine::ModuleManager &moduleManager = shApp.GetModuleManager();

    auto sdl2module = moduleManager.GetById<ShadowEngine::SDL2Module>("module:/platform/sdl2").lock();

    CATCH(initVulkan(sdl2module->window->sdlWindowPtr);)
}

void VulkanModule::Destroy() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void VulkanModule::createAppAndVulkanInstance(bool enableValidation, ValidationAndExtension *validations) {
    VkApplicationInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.pApplicationName = "Sup";
    info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    info.pEngineName = "Infinity Drive";
    info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
#ifdef __APPLE__
    VkFlags instanceFlag = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#else
    VkFlags instanceFlag = 0;
#endif
    instanceInfo.flags = instanceFlag;
    instanceInfo.pApplicationInfo = &info;

    auto sdl = ShadowEngine::ShadowApplication::Get().GetModuleManager().GetById<ShadowEngine::SDL2Module>("module:/platform/sdl2").lock();
    auto extensions = validations->getRequiredExtensions(sdl->window->sdlWindowPtr, true);
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

void VulkanModule::initVulkan(SDL_Window *window) {
    validators = new ValidationAndExtension();

    spdlog::info("Initializing Infinity Drive rendering engine");
    spdlog::default_logger()->set_level(spdlog::level::debug);

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
    VkTools::allocator = allocator;

    this->swapchain = new SwapChain();
    this->swapchain->create(surface);
    spdlog::info("Infinity Drive initialization finished.");
}

void VulkanModule::cleanup() {
    // Wait for the GPU to not be busy
    vkDeviceWaitIdle(VulkanModule::getInstance()->getDevice()->logical);

    swapchain->destroy();

    // Destroy the Vulkan Device
    VulkanModule::getInstance()->getDevice()->destroy();

    // Delete the layer validators.
    validators->destroy(validationRequired, vulkan);

    // Delete the surface and Vulkan instance.
    vkDestroySurfaceKHR(vulkan, surface, nullptr);
    vkDestroyInstance(vulkan, nullptr);

    vmaDestroyAllocator(allocator);

    delete swapchain;
    delete device;
    delete validators;
}

void VulkanModule::Init() {
}
