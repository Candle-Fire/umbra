#include "../inc/EditorWindow.h"
#include "vlkx/vulkan/VulkanModule.h"
#include "core/ShadowApplication.h"
#include "core/SDL2Module.h"

namespace ShadowEngine::Editor {
    SHObject_Base_Impl(EditorWindow)

    MODULE_ENTRY(EditorWindow, EditorWindow)

    void EditorWindow::Init() {
        auto renderer = ShadowEngine::ShadowApplication::Get().GetModuleManager().GetById<VulkanModule>("module:/renderer/vulkan").lock();
        auto sdl = ShadowEngine::ShadowApplication::Get().GetModuleManager().GetById<SDL2Module>("module:/platform/sdl2").lock();

        editorPass = std::make_unique<vlkx::ScreenRenderPassManager>(vlkx::RendererConfig { 1, renderer->getSwapchain()->images, true });
        editorPass->initializeRenderPass();
        renderCommands = std::make_unique<vlkx::RenderCommand>(2);

        IMGUI_CHECKVERSION();
        auto* c = ImGui::CreateContext();
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
        vkCreateDescriptorPool(renderer->getDevice()->logical, &pool_info, VK_NULL_HANDLE, &imGuiPool);

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForVulkan(sdl->window->sdlWindowPtr);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = renderer->getVulkan();
        init_info.PhysicalDevice = renderer->getDevice()->physical;
        init_info.Device = renderer->getDevice()->logical;
        init_info.QueueFamily = renderer->getDevice()->queueData.graphics;
        init_info.Queue = renderer->getDevice()->graphicsQueue;
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = imGuiPool;
        init_info.Subpass = 0;
        init_info.MinImageCount = renderer->getSwapchain()->images.size();
        init_info.ImageCount = renderer->getSwapchain()->images.size();
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = VK_NULL_HANDLE;
        init_info.CheckVkResultFn = nullptr;

        ImGui_ImplVulkan_Init(&init_info, **editorPass->getPass());

        ImGui::SetCurrentContext(c);

        VkTools::immediateExecute([](const VkCommandBuffer &commands) { ImGui_ImplVulkan_CreateFontsTexture(commands); }, renderer->getDevice());

    }

    void EditorWindow::Draw() {
            auto sc = ShadowEngine::ShadowApplication::Get().GetModuleManager().GetById<VulkanModule>("module:/renderer/vulkan").lock();
            vkDeviceWaitIdle(sc->getDevice()->logical);
            // Submit to the screen
            renderCommands->execute(renderCommands->getFrame(), sc->getSwapchain()->swapChain, [](int){},
                              [this](const VkCommandBuffer& commands, int frame) {
                                  editorPass->getPass()->execute(commands,
                                                frame,
                                                {
                                                    // Render ImGUI
                                                    [&](const VkCommandBuffer &commands) {
                                                        ImGui_ImplVulkan_NewFrame();
                                                        ImGui_ImplSDL2_NewFrame();
                                                        ImGui::NewFrame();

                                                        ShadowEngine::ShadowApplication::Get().GetEventBus().fire(SH::Events::EditorRender());

                                                        ImGui::Render();
                                                        ImGuiIO &io = ImGui::GetIO();

                                                        ImGui_ImplVulkan_RenderDrawData(
                                                            ImGui::GetDrawData(),
                                                            commands);

                                                        // Update and Render additional Platform Windows
                                                        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                                                            ImGui::UpdatePlatformWindows();
                                                            ImGui::RenderPlatformWindowsDefault();
                                                        }
                                                    }
                                                });
                              }
            );
    }

    glm::vec2 EditorWindow::GetRenderExtent() { return { 640, 480}; }
}