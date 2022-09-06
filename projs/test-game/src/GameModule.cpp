#include <GameModule.h>
#include "imgui.h"
#include "spdlog/spdlog.h"
#include "vlkx/render/geometry/SingleRenderer.h"
#include "imgui_impl_vulkan.h"
#include "core/ShadowApplication.h"
#include "core/SDL2Module.h"
#include "imgui_impl_sdl.h"
#include "core/Time.h"


#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

// Create the renderer
SingleRenderer object;

// Create the camera
Camera camera;

SHObject_Base_Impl(GameModule)

void GameModule::PreInit() { spdlog::info("Game Module loading.."); }

void GameModule::Init() {

    auto shApp = ShadowEngine::ShadowApplication::Get();

    ShadowEngine::ModuleManager &moduleManager = shApp.GetModuleManager();

    auto sdl2module = moduleManager.GetModuleByType<ShadowEngine::SDL2Module>();
    
    CATCH(VulkanManager::getInstance()->initVulkan(sdl2module->_window->sdlWindowPtr);)

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    VkDescriptorPool imGuiPool;
    VulkanManager* vk = VulkanManager::getInstance();
    VkDescriptorPoolSize pool_sizes[] =
            {
                    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
            };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(vk->getDevice()->logical, &pool_info, VK_NULL_HANDLE, &imGuiPool);

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForVulkan(sdl2module->_window->sdlWindowPtr);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vk->getVulkan();
    init_info.PhysicalDevice = vk->getDevice()->physical;
    init_info.Device = vk->getDevice()->logical;
    init_info.QueueFamily = vk->getDevice()->queueData.graphics;
    init_info.Queue = vk->getDevice()->graphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imGuiPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = vk->getSwapchain()->images.size();
    init_info.ImageCount = vk->getSwapchain()->images.size();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.CheckVkResultFn = nullptr;
    ImGui_ImplVulkan_Init(&init_info, vk->getRenderPass()->pass);

    // Upload Fonts
    {
        // Prepare to create a temporary command pool.
        VkCommandPool pool;
        VkCommandPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.queueFamilyIndex = vk->getDevice()->queueData.graphics;
        poolCreateInfo.flags = 0;

        // Create the pool
        if (vkCreateCommandPool(vk->getDevice()->logical, &poolCreateInfo, nullptr, &pool) != VK_SUCCESS)
            throw std::runtime_error("Unable to allocate a temporary command pool");

        VkCommandBuffer buffer = VkTools::createTempCommandBuffer(pool, vk->getDevice()->logical);

        ImGui_ImplVulkan_CreateFontsTexture(buffer);

        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &buffer;

        VkTools::executeAndDeleteTempBuffer(buffer, pool, vk->getDevice()->graphicsQueue, vk->getDevice()->logical);

        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    CATCH(object.createSingleRenderer(Geo::MeshType::Cube, glm::vec3(-1, 0, -1), glm::vec3(0.5));)
    camera.init(45, 1280, 720, 0.1, 10000);
    camera.setPosition(glm::vec3(0, 0, 4));
}

void GameModule::Update() {
    object.setRotation(glm::rotate(object.getRotation(), (float) 0.5, glm::vec3(1, 0, 0)));
}

void GameModule::PreRender() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void GameModule::Render() {
    bool active = true;
    ImGui::Begin("Game module window", &active, ImGuiWindowFlags_MenuBar);

    ImGui::Text("Such teext from curle's branch");

    ImGui::End();

    CATCH(object.updateUniforms(camera);)
    CATCH(object.draw();)

    bool showDemo = true;
    if (showDemo)
        ImGui::ShowDemoWindow(&showDemo);

}

void GameModule::LateRender() {
    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VulkanManager::getInstance()->getCurrentCommandBuffer());

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void GameModule::AfterFrameEnd() {
    Time::UpdateTime();
}

void GameModule::Destroy() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void GameModule::Event(SDL_Event *) {

}