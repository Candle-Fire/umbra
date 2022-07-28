#include "ShadowApplication.h"

#include "Time.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl.h"
#include <vlkx/vulkan/VulkanManager.h>
#include <vlkx/render/Camera.h>
#include <vlkx/render/geometry/SingleRenderer.h>
#include "spdlog/spdlog.h"

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

namespace ShadowEngine {

    // Create the renderer
    SingleRenderer object;


	ShadowApplication* ShadowApplication::instance = nullptr;
	
	ShadowApplication::ShadowApplication(int argc, char* argv[])
	{
		instance = this;

		if(argc > 1)
		{
			for (size_t i = 0; i < argc; i++)
			{
				std::string param(argv[i]);
				if(param == "-no-gui")
				{
					this->no_gui = true;
				}
                if(param == "-game")
                {
                    this->game = argv[i+1];
                }
			}
		}

		//game = _setupFunc();
	}


	ShadowApplication::~ShadowApplication()
	{
	}

	void ShadowApplication::Init()
	{
        // Initialize SDL. SDL_Init will return -1 if it fails.
        if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
            //std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
            //system("pause");
            // End the program
            //return 1;
        }

        window_ = new ShadowWindow(800,450);

        CATCH(VulkanManager::getInstance()->initVulkan(window_->sdlWindowPtr);)

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

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
        ImGui_ImplSDL2_InitForVulkan(window_->sdlWindowPtr);
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

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
        //IM_ASSERT(font != NULL);

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

        /*
		moduleManager.PushModule(new Log());
		moduleManager.PushModule(new EventSystem::ShadowEventManager());
		moduleManager.PushModule(new SDLPlatform::SDLModule());
		moduleManager.PushModule(new Rendering::Renderer());

		moduleManager.PushModule(new Assets::AssetManager());

		if(!no_gui)
		moduleManager.PushModule(new DebugGui::ImGuiModule());
		
		moduleManager.PushModule(new InputSystem::ShadowActionSystem());
		//moduleManager.PushModule(new Debug::DebugModule());
		moduleManager.PushModule(new EntitySystem::EntitySystem());


		game->Init();
		
		moduleManager.Init();
        */
	}

	void ShadowApplication::Start()
	{
        CATCH(object.createSingleRenderer(Geo::MeshType::Cube, glm::vec3(-1, 0, -1), glm::vec3(0.5));)

        // Create the camera
        Camera camera {};
        camera.init(45, 1280, 720, 0.1, 10000);
        camera.setPosition(glm::vec3(0, 0, 4));

        SDL_Event event;
		while (running)
		{
            while (SDL_PollEvent(&event)) {  // poll until all events are handled!
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT)
                    running = false;
            }

            object.setRotation(glm::rotate(object.getRotation(), (float) 0.5, glm::vec3(1, 0, 0)));

            VulkanManager::getInstance()->startDraw();
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            /** START OF RENDER AREA  */
            /** PUT CODE HERE TO MAKE IT SHOW ON SCREEN */
            CATCH(object.updateUniforms(camera);)
            CATCH(object.draw();)

            bool showDemo = true;
            if (showDemo)
                ImGui::ShowDemoWindow(&showDemo);
            /** END OF RENDER AREA */
            /** CODE AFTER HERE WILL NOT BE SHOWN ON SCREEN */
            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VulkanManager::getInstance()->getCurrentCommandBuffer());
            VulkanManager::getInstance()->endDraw();


            Time::UpdateTime();
		}

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_DestroyWindow(window_->sdlWindowPtr);
        SDL_Quit();
	}
}
