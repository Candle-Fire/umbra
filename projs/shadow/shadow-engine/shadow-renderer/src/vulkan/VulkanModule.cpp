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
bool renderingGeometry;
std::unique_ptr<vlkx::ScreenRenderPassManager> editorPass;

std::unique_ptr<vlkx::RenderCommand> editorRenderCommands;

const std::unique_ptr<vlkx::ScreenRenderPassManager> &VulkanModule::getRenderPass() {
    return renderPass;
}

VulkanModule::~VulkanModule() = default;

VulkanModule *VulkanModule::instance = nullptr;

VulkanModule *VulkanModule::getInstance() {
    return VulkanModule::instance != nullptr ? VulkanModule::instance
                                             : (VulkanModule::instance = new VulkanModule());
}

void VulkanModule::EnableEditor() {
    editorEnabled = true;
}

VkExtent2D VulkanModule::GetRenderExtent() {
    if (editorEnabled) {
        if (renderingGeometry)
            return editorContentFrames[0]->getExtent();
        else
            return swapchain->extent;
    } else
        return swapchain->extent;
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
    editorPass->initializeRenderPass();
}

void VulkanModule::PreInit() {
    spdlog::info("Vulkan Renderer Module loading..");

    auto shApp = ShadowEngine::ShadowApplication::Get();

    ShadowEngine::ModuleManager &moduleManager = shApp.GetModuleManager();

    auto sdl2module = moduleManager.GetById<ShadowEngine::SDL2Module>("module:/platform/sdl2").lock();

    CATCH(initVulkan(sdl2module->window->sdlWindowPtr);)

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    VkDescriptorPool imGuiPool;
    VkDescriptorPoolSize pool_sizes[] =
        {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
        };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t) IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(getDevice()->logical, &pool_info, VK_NULL_HANDLE, &imGuiPool);

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForVulkan(wnd);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = getVulkan();
    init_info.PhysicalDevice = getDevice()->physical;
    init_info.Device = getDevice()->logical;
    init_info.QueueFamily = getDevice()->queueData.graphics;
    init_info.Queue = getDevice()->graphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imGuiPool;
    init_info.Subpass = 1;
    init_info.MinImageCount = getSwapchain()->images.size();
    init_info.ImageCount = getSwapchain()->images.size();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.CheckVkResultFn = nullptr;

    if (editorEnabled) {
        editorContentFrames.resize(1);
        for (size_t i = 0; i < 1; i++) {
            vlkx::TextureImage::Meta meta{
                {nullptr}, {vlkx::ImageUsage::renderTarget(0)}, VK_FORMAT_R8G8B8A8_SRGB, 640, 480, 4
            };
            editorContentFrames[i] = std::make_unique<vlkx::TextureImage>(0, vlkx::ImageSampler::Config{}, meta);
        }

        editorRenderCommands = std::make_unique<vlkx::RenderCommand>(editorContentFrames.size());
    }

    renderPass = std::make_unique<vlkx::ScreenRenderPassManager>(vlkx::RendererConfig{editorEnabled ? 1 : 2,
                                                                                      editorEnabled
                                                                                      ? editorContentFrames
                                                                                      : swapchain->images,
                                                                                      !editorEnabled});
    renderPass->initializeRenderPass();
    editorPass = std::make_unique<vlkx::ScreenRenderPassManager>(vlkx::RendererConfig{2, swapchain->images, true});
    editorPass->initializeRenderPass();

    ImGui_ImplVulkan_Init(&init_info, **(editorEnabled ? editorPass : renderPass)->getPass());

    VkTools::immediateExecute([](const VkCommandBuffer &commands) { ImGui_ImplVulkan_CreateFontsTexture(commands); },
                              getDevice());

    if (editorEnabled) {
        editorRenderPlanes.resize(editorContentFrames.size());
        for (size_t i = 0; i < editorContentFrames.size(); i++) {
            editorRenderPlanes[i] = ImGui_ImplVulkan_AddTexture(VkTools::createSampler(VK_FILTER_LINEAR,
                                                                                       VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                                                       0,
                                                                                       device->logical),
                                                                editorContentFrames[i]->getView(),
                                                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    }

    renderingGeometry = false;
}

void VulkanModule::BeginRenderPass(const std::unique_ptr<vlkx::RenderCommand> &commands) {
    const auto update = !editorEnabled
                        ? [](const int frame) { ShadowEngine::ShadowApplication::Get().GetModuleManager().Update(frame); }
                        : [](const int frame) {};

    const auto res =
        commands->execute(commands->getFrame(), swapchain->swapChain, update,
                          [this](const VkCommandBuffer &buffer, int frame) {
                              (editorEnabled ? editorPass : renderPass)->getPass()
                                  ->execute(buffer,
                                            frame,
                                            {
                                                // Render our models
                                                [this, frame](const VkCommandBuffer &commands) {
                                                    if (!editorEnabled) {
                                                        renderingGeometry =
                                                            true;
                                                        ShadowEngine::ShadowApplication::Get().GetEventBus().fire(SH::Events::Render(const_cast<VkCommandBuffer&>(commands), frame));

                                                        ShadowEngine::ShadowApplication::Get().GetEventBus().fire(SH::Events::PostRender(const_cast<VkCommandBuffer&>(commands), frame));
                                                        renderingGeometry =
                                                            false;
                                                    }
                                                },
                                                // Render ImGUI
                                                [&](const VkCommandBuffer &commands) {
                                                    ImGui_ImplVulkan_NewFrame();
                                                    ImGui_ImplSDL2_NewFrame();
                                                    ImGui::NewFrame();

                                                    ShadowEngine::ShadowApplication::Get().GetEventBus().fire(SH::Events::OverlayRender());

                                                    ImGui::Render();
                                                    ImGuiIO &io = ImGui::GetIO();
                                                    (void) io;

                                                    ImGui_ImplVulkan_RenderDrawData(
                                                        ImGui::GetDrawData(),
                                                        commands);

                                                    // Update and Render additional Platform Windows
                                                    if (io.ConfigFlags
                                                        & ImGuiConfigFlags_ViewportsEnable) {
                                                        ImGui::UpdatePlatformWindows();
                                                        ImGui::RenderPlatformWindowsDefault();
                                                    }
                                                }
                                            });
                          }
        );

    if (res.has_value()) {
        ShadowEngine::ShadowApplication::Get().GetEventBus().fire(SH::Events::Recreate());
    }
}

void VulkanModule::PreRender(SH::Events::PreRender) {
    if (editorEnabled) {
        editorRenderCommands->executeSimple(editorRenderCommands->getFrame(),
                                            [](const int frame) {
                                                ShadowEngine::ShadowApplication::Get().GetModuleManager().Update(frame);
                                            },
                                            [&](const VkCommandBuffer &buffer, int frame) {
                                                renderPass->getPass()->execute(buffer, frame, {
                                                    [&](const VkCommandBuffer &commands) {
                                                        renderingGeometry = true;
                                                        ShadowEngine::ShadowApplication::Get().GetEventBus().fire(SH::Events::Render(const_cast<VkCommandBuffer&>(commands), frame));

                                                        ShadowEngine::ShadowApplication::Get().GetEventBus().fire(SH::Events::PostRender(const_cast<VkCommandBuffer&>(commands), frame));
                                                        renderingGeometry = false;
                                                    }
                                                });
                                            }
        );

    }
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

void VulkanModule::initVulkan(SDL_Window *window) {
    wnd = window;
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

VkDescriptorSet VulkanModule::getEditorRenderPlanes() {
    return editorRenderPlanes[0];
}

void VulkanModule::Init() {
    ShadowEngine::ShadowApplication::Get().GetEventBus()
        .subscribe<SH::Events::PreRender>(std::bind(&VulkanModule::PreRender, this, std::placeholders::_1));
}
